#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Math {

// Package declaration
namespace BoundingBox
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math.BoundingBox" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "BoundingBox", sizeof(cgBoundingBox), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the default constructor, destructor and assignment operators.
            cgScriptInterop::Utils::registerDefaultCDA<cgBoundingBox>( engine, "BoundingBox" );
            BINDSUCCESS( engine->registerObjectBehavior( "BoundingBox", asBEHAVE_CONSTRUCT,  "void f(const Vector3&, const Vector3&)", asFUNCTIONPR(constructBoundingBox,(const cgVector3&, const cgVector3&, cgBoundingBox*),void), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "BoundingBox", asBEHAVE_CONSTRUCT,  "void f(float, float, float, float, float, float)", asFUNCTIONPR(constructBoundingBox,(cgFloat, cgFloat, cgFloat, cgFloat, cgFloat, cgFloat,cgBoundingBox*),void), asCALL_CDECL_OBJLAST) );
            
            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "BoundingBox &opAddAssign(const Vector3 &in)", asMETHODPR(cgBoundingBox, operator+=, (const cgVector3&), cgBoundingBox&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "BoundingBox &opSubAssign(const Vector3 &in)", asMETHODPR(cgBoundingBox, operator-=, (const cgVector3&), cgBoundingBox&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "BoundingBox &opMulAssign(float)", asMETHODPR(cgBoundingBox, operator*=, (cgFloat), cgBoundingBox&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "BoundingBox &opMulAssign(const Transform&)", asMETHODPR(cgBoundingBox, operator*=, (const cgTransform&), cgBoundingBox&), asCALL_THISCALL) );

            // Register the global operator overloads
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "BoundingBox opMul(float) const", asMETHODPR(cgBoundingBox, operator*, (cgFloat) const, cgBoundingBox), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool opEquals(const BoundingBox &in) const", asMETHODPR(cgBoundingBox, operator==, (const cgBoundingBox &) const, bool), asCALL_THISCALL) );

            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "BoundingBox", "Vector3 min", offsetof(cgBoundingBox,min) ) );
            BINDSUCCESS( engine->registerObjectProperty( "BoundingBox", "Vector3 max", offsetof(cgBoundingBox,max) ) );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "Vector3 getDimensions( ) const", asMETHODPR(cgBoundingBox,getDimensions,( ) const, cgVector3), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "Vector3 getCenter( ) const", asMETHODPR(cgBoundingBox,getCenter,( ) const, cgVector3), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "Vector3 getExtents( ) const", asMETHODPR(cgBoundingBox,getExtents,( ) const, cgVector3), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "Plane getPlane( VolumePlane ) const", asMETHODPR(cgBoundingBox,getPlane,( cgVolumePlane::Side ) const, cgPlane), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "BoundingBox & addPoint( const Vector3 &in )", asMETHODPR(cgBoundingBox,addPoint,( const cgVector3& ), cgBoundingBox& ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool intersect( const BoundingBox &in ) const", asMETHODPR(cgBoundingBox,intersect,( const cgBoundingBox& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool intersect( const BoundingBox &in, bool &inout ) const", asMETHODPR(cgBoundingBox,intersect,( const cgBoundingBox&, bool& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool intersect( const BoundingBox &in, BoundingBox &inout ) const", asMETHODPR(cgBoundingBox,intersect,( const cgBoundingBox&, cgBoundingBox& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool intersect( const BoundingBox &in, const Vector3 &in ) const", asMETHODPR(cgBoundingBox,intersect,( const cgBoundingBox&, const cgVector3& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool intersect( const Vector3 &in, const Vector3 &in, float &inout, bool ) const", asMETHODPR(cgBoundingBox,intersect,( const cgVector3&, const cgVector3&, cgFloat&, bool ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool intersect( const Vector3 &in, const Vector3 &in, const Vector3&, const BoundingBox& ) const", asMETHODPR(cgBoundingBox,intersect,( const cgVector3&, const cgVector3&, const cgVector3&, const cgBoundingBox& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool containsPoint( const Vector3 &in ) const", asMETHODPR(cgBoundingBox,containsPoint,( const cgVector3& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool containsPoint( const Vector3 &in, const Vector3 &in ) const", asMETHODPR(cgBoundingBox,containsPoint,( const cgVector3&, const cgVector3& ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool containsPoint( const Vector3 &in, float ) const", asMETHODPR(cgBoundingBox,containsPoint,( const cgVector3&, cgFloat ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "Vector3 closestPoint( const Vector3 &in ) const", asMETHODPR(cgBoundingBox,closestPoint,( const cgVector3& ) const, cgVector3 ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "void validate( )", asMETHODPR(cgBoundingBox,validate,( ), void ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "void reset( )", asMETHODPR(cgBoundingBox,reset,( ), void ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "BoundingBox & transform( const Transform &in )", asMETHODPR(cgBoundingBox,transform,( const cgTransform& ), cgBoundingBox& ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "void inflate( float )", asMETHODPR(cgBoundingBox,inflate,( cgFloat ), void ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "void inflate( const Vector3 &in )", asMETHODPR(cgBoundingBox,inflate,( const cgVector3& ), void ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool isPopulated( ) const", asMETHODPR(cgBoundingBox,isPopulated,( ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "bool isDegenerate( ) const", asMETHODPR(cgBoundingBox,isDegenerate,( ) const, bool ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float x( ) const", asMETHODPR(cgBoundingBox,x,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float y( ) const", asMETHODPR(cgBoundingBox,y,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float z( ) const", asMETHODPR(cgBoundingBox,z,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float width( ) const", asMETHODPR(cgBoundingBox,width,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float height( ) const", asMETHODPR(cgBoundingBox,height,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float depth( ) const", asMETHODPR(cgBoundingBox,depth,( ) const, cgFloat ), asCALL_THISCALL ) );
            
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float get_x( ) const", asMETHODPR(cgBoundingBox,x,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float get_y( ) const", asMETHODPR(cgBoundingBox,y,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float get_z( ) const", asMETHODPR(cgBoundingBox,z,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float get_width( ) const", asMETHODPR(cgBoundingBox,width,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float get_height( ) const", asMETHODPR(cgBoundingBox,height,( ) const, cgFloat ), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectMethod( "BoundingBox", "float get_depth( ) const", asMETHODPR(cgBoundingBox,depth,( ) const, cgFloat ), asCALL_THISCALL ) );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructBoundingBox()
        /// <summary>
        /// This is a wrapper for the default cgBoundingBox constructor, since it is
        /// not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructBoundingBox( cgFloat minX, cgFloat minY, cgFloat minZ, cgFloat maxX, cgFloat maxY, cgFloat maxZ, cgBoundingBox *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgBoundingBox( minX, minY, minZ, maxX, maxY, maxZ );
        }

        //-----------------------------------------------------------------------------
        //  Name : constructBoundingBox ()
        /// <summary>
        /// This is a wrapper for the default cgBoundingBox constructor, since it is
        /// not possible to take the address of the constructor directly.
        /// </summary>
        //-----------------------------------------------------------------------------
        static void constructBoundingBox( const cgVector3 & min, const cgVector3 & max, cgBoundingBox *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgBoundingBox( min, max );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Math::BoundingBox