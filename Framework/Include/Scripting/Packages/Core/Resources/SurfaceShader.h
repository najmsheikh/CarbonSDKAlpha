#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgSurfaceShader.h>
#include "../../../../../Lib/AngelScript/source/as_tokendef.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace SurfaceShader
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.SurfaceShader" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "SurfaceShader", 0, asOBJ_REF ) );
            BINDSUCCESS( engine->registerObjectType( "SurfaceShaderHandle", sizeof(cgSurfaceShaderHandle), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerInterface("ISurfaceShader" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgSurfaceShader>( engine );

            // Register base / system behaviors.
            Core::Resources::Resource::registerResourceMethods<cgSurfaceShader>( engine, "SurfaceShader" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const VertexShaderHandle &in )", 
                asMETHODPR(cgSurfaceShader,selectVertexShader,(const cgVertexShaderHandle&),bool), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in )", 
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString& ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in )", 
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9 ),bool), asCALL_CDECL_OBJFIRST) );

            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in )", 
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString& ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in )", 
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "VertexShaderHandle getVertexShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getVertexShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9 ),cgVertexShaderHandle), asCALL_CDECL_OBJFIRST) );
            
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const PixelShaderHandle &in )", 
                asMETHODPR(cgSurfaceShader,selectPixelShader,(const cgPixelShaderHandle&),bool), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in )", 
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString& ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in )", 
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11,void* p12,cgInt t12 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11,void* p12,cgInt t12,void* p13,cgInt t13 ),bool), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(selectPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11,void* p12,cgInt t12,void* p13,cgInt t13,void* p14,cgInt t14 ),bool), asCALL_CDECL_OBJFIRST) );

            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in )", 
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString& ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in )", 
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )", 
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11,void* p12,cgInt t12 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11,void* p12,cgInt t12,void* p13,cgInt t13 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "PixelShaderHandle getPixelShader( const String &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in, const ? &in )",
                asFUNCTIONPR(getPixelShader,(cgSurfaceShader*,const cgString&,void* p0,cgInt t0,void* p1,cgInt t1,void* p2,cgInt t2,void* p3,cgInt t3,void* p4,cgInt t4,void* p5,cgInt t5,void* p6,cgInt t6,void* p7,cgInt t7,void* p8,cgInt t8,void* p9,cgInt t9,void* p10,cgInt t10,void* p11,cgInt t11,void* p12,cgInt t12,void* p13,cgInt t13,void* p14,cgInt t14 ),cgPixelShaderHandle), asCALL_CDECL_OBJFIRST) );

            // Script 'null' handle support
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectVertexShader( NullHandle@ )", asFUNCTIONPR(selectVertexShader,( void*, cgSurfaceShader* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectPixelShader( NullHandle@ )", asFUNCTIONPR(selectPixelShader,( void*, cgSurfaceShader* ), bool), asCALL_CDECL_OBJLAST ) );

            // Techniques
            // ToDo: getTechnique now returns a void pointer
            //BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "int getTechnique( const String &in )", asMETHODPR(cgSurfaceShader,getTechnique,(const cgString&),cgInt32), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectTechnique( const String &in )", asMETHODPR(cgSurfaceShader,selectTechnique,(const cgString&),bool), asCALL_THISCALL ) );
            // ToDo: selectTechnique now accepts a void pointer
            //BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool selectTechnique( int )", asMETHODPR(cgSurfaceShader,SelectTechnique,(cgInt32),bool), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool beginTechnique( )", asMETHODPR(cgSurfaceShader,beginTechnique,(),bool), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool beginTechnique( const String &in )", asMETHODPR(cgSurfaceShader,beginTechnique,(const cgString&),bool), asCALL_THISCALL ) );
            // ToDo: beginTechnique now accepts a void pointer
            //BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool beginTechnique( int )", asMETHODPR(cgSurfaceShader,beginTechnique,(cgInt32),bool), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool commitChanges( )", asMETHODPR(cgSurfaceShader,commitChanges,(),bool), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "TechniqueResult executeTechniquePass( )", asMETHODPR(cgSurfaceShader,executeTechniquePass,(),cgTechniqueResult::Base), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "void endTechnique( )", asMETHODPR(cgSurfaceShader,endTechnique,(),void), asCALL_THISCALL ) );        

            // Descriptors
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool getConstantBufferDesc( int, ConstantBufferDesc &inout ) const", asMETHODPR(cgSurfaceShader,getConstantBufferDesc,( cgInt32, cgConstantBufferDesc& ) const,bool), asCALL_THISCALL ) );        
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool getConstantBufferDesc( const String &in, ConstantBufferDesc &inout ) const", asMETHODPR(cgSurfaceShader,getConstantBufferDesc,( const cgString&, cgConstantBufferDesc& ) const,bool), asCALL_THISCALL ) );        
            BINDSUCCESS( engine->registerObjectMethod( "SurfaceShader", "bool getConstantBufferDesc( const String &in, const String &in, ConstantBufferDesc &inout ) const", asMETHODPR(cgSurfaceShader,getConstantBufferDesc,( const cgString&, const cgString&, cgConstantBufferDesc& ) const,bool), asCALL_THISCALL ) );

            // Register system internal global functions.
            BINDSUCCESS( engine->registerGlobalFunction( "void __sh_push_code_output( const String &in )", asFUNCTION( cgSurfaceShader::pushCodeOutput ), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "void __sh_pop_code_output( )", asFUNCTION( cgSurfaceShader::popCodeOutput ), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "String __sh_generate_shader_call( const String &in, const String &in, const String &in, const String &in)", asFUNCTION( cgSurfaceShader::generateShaderCall ), asCALL_CDECL ) );
            BINDSUCCESS( engine->registerGlobalFunction( "void __sh_code( const String &in )", asFUNCTION( cgSurfaceShader::emitCode ), asCALL_CDECL ) );

            // Register resource handle type
            Core::Resources::Types::registerResourceHandleMethods<cgSurfaceShaderHandle>( engine, "SurfaceShaderHandle", "SurfaceShader" );
        }

        inline static cgScriptArgumentType::Base translateArgType( size_t i )
        {
            static const cgScriptArgumentType::Base ArgTypes[] = 
            {
                cgScriptArgumentType::None, cgScriptArgumentType::Bool, cgScriptArgumentType::Byte, cgScriptArgumentType::Word,
                cgScriptArgumentType::DWord, cgScriptArgumentType::QWord, cgScriptArgumentType::Byte, cgScriptArgumentType::Word,
                cgScriptArgumentType::DWord, cgScriptArgumentType::QWord, cgScriptArgumentType::Float, cgScriptArgumentType::Double
            };
            return ArgTypes[i];
        }
        inline static const cgString & translateArgTypeName( size_t i )
        {
            static const cgString ArgTypeNames[] =
            {
                cgString::Empty, _T("bool"), _T("int8"), _T("int16"), _T("int32"), _T("int64"), _T("uint8"), _T("uint16"),
                _T("uint32"), _T("uint64"), _T("float"), _T("double")
            };
            return ArgTypeNames[i];
        }

        //---------------------------------------------------------------------
        //  Name : selectVertexShader () (Static, Private)
        /// <summary>
        /// Various wrapper functions for vertex shader selection on behalf of
        /// the surface shader. These provide support for variable argument
        /// types.
        /// </summary>
        //---------------------------------------------------------------------
        static bool selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name )
        {
            return selectVertexShader( thisPointer, name, CG_NULL, CG_NULL, 0 );
        }

        static bool selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0 )
        {
            void * const arguments[]  = {p0};
            const cgInt  types[] = {t0};
            return selectVertexShader( thisPointer, name, arguments, types, 1 );
        }

        static bool selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1 )
        {
            void * const arguments[]  = {p0,p1};
            const cgInt  types[] = {t0,t1};
            return selectVertexShader( thisPointer, name, arguments, types, 2 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2 )
        {
            void * const arguments[]  = {p0,p1,p2};
            const cgInt  types[] = {t0,t1,t2};
            return selectVertexShader( thisPointer, name, arguments, types, 3 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3 )
        {
            void * const arguments[]  = {p0,p1,p2,p3};
            const cgInt  types[] = {t0,t1,t2,t3};
            return selectVertexShader( thisPointer, name, arguments, types, 4 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4};
            const cgInt  types[] = {t0,t1,t2,t3,t4};
            return selectVertexShader( thisPointer, name, arguments, types, 5 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5 )
        {
            void * const arguments[] = {p0,p1,p2,p3,p4,p5};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5};
            return selectVertexShader( thisPointer, name, arguments, types, 6 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6};
            return selectVertexShader( thisPointer, name, arguments, types, 7 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7};
            return selectVertexShader( thisPointer, name, arguments, types, 8 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8};
            return selectVertexShader( thisPointer, name, arguments, types, 9 );
        }

        static bool  selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9};
            return selectVertexShader( thisPointer, name, arguments, types, 10 );
        }

        static bool selectVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * const p[], const cgInt t[], cgInt argumentCount )
        {
            // Retrieve the scripting engine.
            cgScriptEngine  * engine = cgScriptEngine::getInstance();
            asIScriptEngine * internalEngine = engine->getInternalEngine();

            // Populate script argument array.
            static cgScriptArgument::Array arguments;
            arguments.resize( argumentCount );
            for ( cgInt i = 0; i < argumentCount; ++i )
            {
                // Primitive type?
                if ( !(t[i] & asTYPEID_MASK_OBJECT) )
                {
                    arguments[i].data = p[i];
                    arguments[i].type  = translateArgType(t[i]);
                    arguments[i].declaration = translateArgTypeName(t[i]);

                } // End if primitive
                else
                {
                    // What is the data type?
                    asIObjectType * objectType = internalEngine->GetObjectTypeById( t[i] );
                    
                    // Object handle or template type?
                    if ( (objectType->GetFlags() & asOBJ_REF) || (objectType->GetFlags() & asOBJ_TEMPLATE) )
                    {
                        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to selectVertexShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                        return false;

                    } // End if unsupported
                    else
                    {
                        // Array type?
                        if ( std::string(objectType->GetName()) != "array" )
                        {
                            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to selectVertexShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                            return false;
                        
                        } // End if !array
                        arguments[i].type = cgScriptArgumentType::Array;
                        arguments[i].data = p[i];
                        
                        // Determine the subtype. Note: This relies upon a modified version
                        // of angelscript in which a new "asCObjectType::GetSubTypeToken()"
                        // method is supplied.
                        int subTypeToken = objectType->GetSubTypeToken();
                        switch ( subTypeToken )
                        {
                            case ttBool:
                                arguments[i].subType        = cgScriptArgumentType::Bool;
                                arguments[i].declaration = _T("const array<bool>&");
                                break;
                            case ttInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<int8>&");
                                break;
                            case ttUInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<uint8>&");
                                break;
                            case ttInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<int16>&");
                                break;
                            case ttUInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<uint16>&");
                                break;
                            case ttInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<int>&");
                                break;
                            case ttUInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<uint>&");
                                break;
                            case ttInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<int64>&");
                                break;
                            case ttUInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<uint64>&");
                                break;
                            case ttFloat:
                                arguments[i].subType        = cgScriptArgumentType::Float;
                                arguments[i].declaration = _T("const array<float>&");
                                break;
                            case ttDouble:
                                arguments[i].subType        = cgScriptArgumentType::Double;
                                arguments[i].declaration = _T("const array<double>&");
                                break;
                            default:
                                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to selectVertexShader provided an argument with an unsupported array type. Only arrays of primitive value types are supported.\n") );
                                return false;

                        } // End switch size
                        
                    } // End if value

                } // End if !primitive

            } // Next Argument

            // Pass through to surface shader
            return thisPointer->selectVertexShader( name, arguments );
        }

        //---------------------------------------------------------------------
        //  Name : getVertexShader () (Static, Private)
        /// <summary>
        /// Various wrapper functions for vertex shader selection on behalf of
        /// the surface shader. These provide support for variable argument
        /// types.
        /// </summary>
        //---------------------------------------------------------------------
        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name )
        {
            return getVertexShader( thisPointer, name, CG_NULL, CG_NULL, 0 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0 )
        {
            void * const arguments[]  = {p0};
            const cgInt  types[] = {t0};
            return getVertexShader( thisPointer, name, arguments, types, 1 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1 )
        {
            void * const arguments[]  = {p0,p1};
            const cgInt  types[] = {t0,t1};
            return getVertexShader( thisPointer, name, arguments, types, 2 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2 )
        {
            void * const arguments[]  = {p0,p1,p2};
            const cgInt  types[] = {t0,t1,t2};
            return getVertexShader( thisPointer, name, arguments, types, 3 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3 )
        {
            void * const arguments[]  = {p0,p1,p2,p3};
            const cgInt  types[] = {t0,t1,t2,t3};
            return getVertexShader( thisPointer, name, arguments, types, 4 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4};
            const cgInt  types[] = {t0,t1,t2,t3,t4};
            return getVertexShader( thisPointer, name, arguments, types, 5 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5 )
        {
            void * const arguments[] = {p0,p1,p2,p3,p4,p5};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5};
            return getVertexShader( thisPointer, name, arguments, types, 6 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6};
            return getVertexShader( thisPointer, name, arguments, types, 7 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7};
            return getVertexShader( thisPointer, name, arguments, types, 8 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8};
            return getVertexShader( thisPointer, name, arguments, types, 9 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9};
            return getVertexShader( thisPointer, name, arguments, types, 10 );
        }

        static cgVertexShaderHandle getVertexShader( cgSurfaceShader * thisPointer, const cgString & name, void * const p[], const cgInt t[], cgInt argumentCount )
        {
            // Retrieve the scripting engine.
            cgScriptEngine  * engine = cgScriptEngine::getInstance();
            asIScriptEngine * internalEngine = engine->getInternalEngine();

            // Populate script argument array.
            static cgScriptArgument::Array arguments;
            arguments.resize( argumentCount );
            for ( cgInt i = 0; i < argumentCount; ++i )
            {
                // Primitive type?
                if ( !(t[i] & asTYPEID_MASK_OBJECT) )
                {
                    arguments[i].data = p[i];
                    arguments[i].type  = translateArgType(t[i]);
                    arguments[i].declaration = translateArgTypeName(t[i]);

                } // End if primitive
                else
                {
                    // What is the data type?
                    asIObjectType * objectType = internalEngine->GetObjectTypeById( t[i] );
                    
                    // Object handle or template type?
                    if ( (objectType->GetFlags() & asOBJ_REF) || (objectType->GetFlags() & asOBJ_TEMPLATE) )
                    {
                        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to getVertexShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                        return false;

                    } // End if unsupported
                    else
                    {
                        // Array type?
                        if ( std::string(objectType->GetName()) != "array" )
                        {
                            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to getVertexShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                            return false;
                        
                        } // End if !array
                        arguments[i].type = cgScriptArgumentType::Array;
                        arguments[i].data = p[i];
                        
                        // Determine the subtype. Note: This relies upon a modified version
                        // of angelscript in which a new "asCObjectType::GetSubTypeToken()"
                        // method is supplied.
                        int subTypeToken = objectType->GetSubTypeToken();
                        switch ( subTypeToken )
                        {
                            case ttBool:
                                arguments[i].subType        = cgScriptArgumentType::Bool;
                                arguments[i].declaration = _T("const array<bool>&");
                                break;
                            case ttInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<int8>&");
                                break;
                            case ttUInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<uint8>&");
                                break;
                            case ttInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<int16>&");
                                break;
                            case ttUInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<uint16>&");
                                break;
                            case ttInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<int>&");
                                break;
                            case ttUInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<uint>&");
                                break;
                            case ttInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<int64>&");
                                break;
                            case ttUInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<uint64>&");
                                break;
                            case ttFloat:
                                arguments[i].subType        = cgScriptArgumentType::Float;
                                arguments[i].declaration = _T("const array<float>&");
                                break;
                            case ttDouble:
                                arguments[i].subType        = cgScriptArgumentType::Double;
                                arguments[i].declaration = _T("const array<double>&");
                                break;
                            default:
                                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to getVertexShader provided an argument with an unsupported array type. Only arrays of primitive value types are supported.\n") );
                                return false;

                        } // End switch size
                    
                    } // End if value

                } // End if !primitive

            } // Next Argument

            // Pass through to surface shader
            return thisPointer->getVertexShader( name, arguments );
        }

        //---------------------------------------------------------------------
        //  Name : selectPixelShader () (Static, Private)
        /// <summary>
        /// Various wrapper functions for pixel shader selection on behalf of
        /// the surface shader. These provide support for variable argument
        /// types.
        /// </summary>
        //---------------------------------------------------------------------
        static bool selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name )
        {
            return selectPixelShader( thisPointer, name, CG_NULL, CG_NULL, 0 );
        }

        static bool selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0 )
        {
            void * const arguments[]  = {p0};
            const cgInt  types[] = {t0};
            return selectPixelShader( thisPointer, name, arguments, types, 1 );
        }

        static bool selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1 )
        {
            void * const arguments[]  = {p0,p1};
            const cgInt  types[] = {t0,t1};
            return selectPixelShader( thisPointer, name, arguments, types, 2 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2 )
        {
            void * const arguments[]  = {p0,p1,p2};
            const cgInt  types[] = {t0,t1,t2};
            return selectPixelShader( thisPointer, name, arguments, types, 3 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3 )
        {
            void * const arguments[]  = {p0,p1,p2,p3};
            const cgInt  types[] = {t0,t1,t2,t3};
            return selectPixelShader( thisPointer, name, arguments, types, 4 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4};
            const cgInt  types[] = {t0,t1,t2,t3,t4};
            return selectPixelShader( thisPointer, name, arguments, types, 5 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5 )
        {
            void * const arguments[] = {p0,p1,p2,p3,p4,p5};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5};
            return selectPixelShader( thisPointer, name, arguments, types, 6 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6};
            return selectPixelShader( thisPointer, name, arguments, types, 7 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7};
            return selectPixelShader( thisPointer, name, arguments, types, 8 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8};
            return selectPixelShader( thisPointer, name, arguments, types, 9 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9};
            return selectPixelShader( thisPointer, name, arguments, types, 10 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10};
            return selectPixelShader( thisPointer, name, arguments, types, 11 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11};
            return selectPixelShader( thisPointer, name, arguments, types, 12 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11, void * p12, cgInt t12 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12};
            return selectPixelShader( thisPointer, name, arguments, types, 13 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11, void * p12, cgInt t12, void * p13, cgInt t13 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13};
            return selectPixelShader( thisPointer, name, arguments, types, 14 );
        }

        static bool  selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11, void * p12, cgInt t12, void * p13, cgInt t13, void * p14, cgInt t14 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14};
            return selectPixelShader( thisPointer, name, arguments, types, 15 );
        }

        static bool selectPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * const p[], const cgInt t[], cgInt argumentCount )
        {
            // Retrieve the scripting engine.
            cgScriptEngine  * engine = cgScriptEngine::getInstance();
            asIScriptEngine * internalEngine = engine->getInternalEngine();

            // Populate script argument array.
            static cgScriptArgument::Array arguments;
            arguments.resize( argumentCount );
            for ( cgInt i = 0; i < argumentCount; ++i )
            {
                // Primitive type?
                if ( !(t[i] & asTYPEID_MASK_OBJECT) )
                {
                    arguments[i].data = p[i];
                    arguments[i].type  = translateArgType(t[i]);
                    arguments[i].declaration = translateArgTypeName(t[i]);

                } // End if primitive
                else
                {
                    // What is the data type?
                    asIObjectType * objectType = internalEngine->GetObjectTypeById( t[i] );
                    
                    // Object handle or template type?
                    if ( (objectType->GetFlags() & asOBJ_REF) || (objectType->GetFlags() & asOBJ_TEMPLATE) )
                    {
                        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to selectPixelShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                        return false;

                    } // End if unsupported
                    else
                    {
                        // Array type?
                        if ( std::string(objectType->GetName()) != "array" )
                        {
                            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to selectPixelShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                            return false;

                        } // End if !array
                        arguments[i].type = cgScriptArgumentType::Array;
                        arguments[i].data = p[i];
                        
                        // Determine the subtype. Note: This relies upon a modified version
                        // of angelscript in which a new "asCObjectType::GetSubTypeToken()"
                        // method is supplied.
                        int subTypeToken = objectType->GetSubTypeToken();
                        switch ( subTypeToken )
                        {
                            case ttBool:
                                arguments[i].subType        = cgScriptArgumentType::Bool;
                                arguments[i].declaration = _T("const array<bool>&");
                                break;
                            case ttInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<int8>&");
                                break;
                            case ttUInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<uint8>&");
                                break;
                            case ttInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<int16>&");
                                break;
                            case ttUInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<uint16>&");
                                break;
                            case ttInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<int>&");
                                break;
                            case ttUInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<uint>&");
                                break;
                            case ttInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<int64>&");
                                break;
                            case ttUInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<uint64>&");
                                break;
                            case ttFloat:
                                arguments[i].subType        = cgScriptArgumentType::Float;
                                arguments[i].declaration = _T("const array<float>&");
                                break;
                            case ttDouble:
                                arguments[i].subType        = cgScriptArgumentType::Double;
                                arguments[i].declaration = _T("const array<double>&");
                                break;
                            default:
                                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to selectPixelShader provided an argument with an unsupported array type. Only arrays of primitive value types are supported.\n") );
                                return false;

                        } // End switch size
                    
                    } // End if value

                } // End if !primitive

            } // Next Argument

            // Pass through to surface shader
            return thisPointer->selectPixelShader( name, arguments );
        }

        //---------------------------------------------------------------------
        //  Name : getPixelShader () (Static, Private)
        /// <summary>
        /// Various wrapper functions for pixel shader selection on behalf of
        /// the surface shader. These provide support for variable argument
        /// types.
        /// </summary>
        //---------------------------------------------------------------------
        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name )
        {
            return getPixelShader( thisPointer, name, CG_NULL, CG_NULL, 0 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0 )
        {
            void * const arguments[]  = {p0};
            const cgInt  types[] = {t0};
            return getPixelShader( thisPointer, name, arguments, types, 1 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1 )
        {
            void * const arguments[]  = {p0,p1};
            const cgInt  types[] = {t0,t1};
            return getPixelShader( thisPointer, name, arguments, types, 2 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2 )
        {
            void * const arguments[]  = {p0,p1,p2};
            const cgInt  types[] = {t0,t1,t2};
            return getPixelShader( thisPointer, name, arguments, types, 3 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3 )
        {
            void * const arguments[]  = {p0,p1,p2,p3};
            const cgInt  types[] = {t0,t1,t2,t3};
            return getPixelShader( thisPointer, name, arguments, types, 4 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4};
            const cgInt  types[] = {t0,t1,t2,t3,t4};
            return getPixelShader( thisPointer, name, arguments, types, 5 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5 )
        {
            void * const arguments[] = {p0,p1,p2,p3,p4,p5};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5};
            return getPixelShader( thisPointer, name, arguments, types, 6 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6};
            return getPixelShader( thisPointer, name, arguments, types, 7 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7};
            return getPixelShader( thisPointer, name, arguments, types, 8 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8};
            return getPixelShader( thisPointer, name, arguments, types, 9 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9};
            return getPixelShader( thisPointer, name, arguments, types, 10 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10};
            return getPixelShader( thisPointer, name, arguments, types, 11 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11};
            return getPixelShader( thisPointer, name, arguments, types, 12 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11, void * p12, cgInt t12 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12};
            return getPixelShader( thisPointer, name, arguments, types, 13 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11, void * p12, cgInt t12, void * p13, cgInt t13 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13};
            return getPixelShader( thisPointer, name, arguments, types, 14 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * p0, cgInt t0, void * p1, cgInt t1, void * p2, cgInt t2, void * p3, cgInt t3, void * p4, cgInt t4, void * p5, cgInt t5, void * p6, cgInt t6, void * p7, cgInt t7, void * p8, cgInt t8, void * p9, cgInt t9, void * p10, cgInt t10, void * p11, cgInt t11, void * p12, cgInt t12, void * p13, cgInt t13, void * p14, cgInt t14 )
        {
            void * const arguments[]  = {p0,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14};
            const cgInt  types[] = {t0,t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11,t12,t13,t14};
            return getPixelShader( thisPointer, name, arguments, types, 15 );
        }

        static cgPixelShaderHandle getPixelShader( cgSurfaceShader * thisPointer, const cgString & name, void * const p[], const cgInt t[], cgInt argumentCount )
        {
            // Retrieve the scripting engine.
            cgScriptEngine  * engine = cgScriptEngine::getInstance();
            asIScriptEngine * internalEngine = engine->getInternalEngine();

            // Populate script argument array.
            static cgScriptArgument::Array arguments;
            arguments.resize( argumentCount );
            for ( cgInt i = 0; i < argumentCount; ++i )
            {
                // Primitive type?
                if ( !(t[i] & asTYPEID_MASK_OBJECT) )
                {
                    arguments[i].data = p[i];
                    arguments[i].type  = translateArgType(t[i]);
                    arguments[i].declaration = translateArgTypeName(t[i]);

                } // End if primitive
                else
                {
                    // What is the data type?
                    asIObjectType * objectType = internalEngine->GetObjectTypeById( t[i] );
                    
                    // Object handle or template type?
                    if ( (objectType->GetFlags() & asOBJ_REF) || (objectType->GetFlags() & asOBJ_TEMPLATE) )
                    {
                        cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to getPixelShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                        return false;

                    } // End if unsupported
                    else
                    {
                        // Array type?
                        if ( std::string(objectType->GetName()) != "array" )
                        {
                            cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to getPixelShader provided an argument with an unsupported type. Only primitive value types and arrays of primitive values types are supported.\n") );
                            return false;

                        } // End if !array
                        arguments[i].type = cgScriptArgumentType::Array;
                        arguments[i].data = p[i];
                        
                        // Determine the subtype. Note: This relies upon a modified version
                        // of angelscript in which a new "asCObjectType::GetSubTypeToken()"
                        // method is supplied.
                        int subTypeToken = objectType->GetSubTypeToken();
                        switch ( subTypeToken )
                        {
                            case ttBool:
                                arguments[i].subType        = cgScriptArgumentType::Bool;
                                arguments[i].declaration = _T("const array<bool>&");
                                break;
                            case ttInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<int8>&");
                                break;
                            case ttUInt8:
                                arguments[i].subType        = cgScriptArgumentType::Byte;
                                arguments[i].declaration = _T("const array<uint8>&");
                                break;
                            case ttInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<int16>&");
                                break;
                            case ttUInt16:
                                arguments[i].subType        = cgScriptArgumentType::Word;
                                arguments[i].declaration = _T("const array<uint16>&");
                                break;
                            case ttInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<int>&");
                                break;
                            case ttUInt:
                                arguments[i].subType        = cgScriptArgumentType::DWord;
                                arguments[i].declaration = _T("const array<uint>&");
                                break;
                            case ttInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<int64>&");
                                break;
                            case ttUInt64:
                                arguments[i].subType        = cgScriptArgumentType::QWord;
                                arguments[i].declaration = _T("const array<uint64>&");
                                break;
                            case ttFloat:
                                arguments[i].subType        = cgScriptArgumentType::Float;
                                arguments[i].declaration = _T("const array<float>&");
                                break;
                            case ttDouble:
                                arguments[i].subType        = cgScriptArgumentType::Double;
                                arguments[i].declaration = _T("const array<double>&");
                                break;
                            default:
                                cgAppLog::write( cgAppLog::Debug | cgAppLog::Warning, _T("Call to getPixelShader provided an argument with an unsupported array type. Only arrays of primitive value types are supported.\n") );
                                return false;

                        } // End switch size
                    
                    } // End if value

                } // End if !primitive

            } // Next Argument

            // Pass through to surface shader
            return thisPointer->getPixelShader( name, arguments );
        }

        //---------------------------------------------------------------------
        //  Name : selectVertexShader ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// SurfaceShader::selectVertexShader() method that allows the script
        /// to pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool selectVertexShader( void*nullHandle, cgSurfaceShader *thisPointer )
        {
            return thisPointer->selectVertexShader( cgVertexShaderHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : selectPixelShader ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// SurfaceShader::selectPixelShader() method that allows the script
        /// to pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool selectPixelShader( void*nullHandle, cgSurfaceShader *thisPointer )
        {
            return thisPointer->selectPixelShader( cgPixelShaderHandle::Null );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::SurfaceShader