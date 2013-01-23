#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Math/cgMathUtility.h>

// Child packages
#include "Math/Color.h"
#include "Math/Matrix.h"
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Plane.h"
#include "Math/Quaternion.h"
#include "Math/Transform.h"
#include "Math/BoundingBox.h"
#include "Math/Types.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace Math
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Math" )
            DECLARE_PACKAGE_CHILD( Color )
            DECLARE_PACKAGE_CHILD( Matrix )
            DECLARE_PACKAGE_CHILD( Vector2 )
            DECLARE_PACKAGE_CHILD( Vector3 )
            DECLARE_PACKAGE_CHILD( Vector4 )
            DECLARE_PACKAGE_CHILD( Plane )
            DECLARE_PACKAGE_CHILD( Quaternion )
            DECLARE_PACKAGE_CHILD( Transform )
            DECLARE_PACKAGE_CHILD( BoundingBox )
            DECLARE_PACKAGE_CHILD( Types )
        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Global functions
            BINDSUCCESS( engine->registerGlobalFunction("const int32& min( const int32 &in, const int32 &in )", asFUNCTIONPR(std::min<cgInt32>,(const cgInt32&, const cgInt32&), const cgInt32&), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("const int32& max( const int32 &in, const int32 &in )", asFUNCTIONPR(std::max<cgInt32>,(const cgInt32&, const cgInt32&), const cgInt32&), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("const float& min( const float &in, const float &in )", asFUNCTIONPR(std::min<cgFloat>,(const cgFloat&, const cgFloat&), const cgFloat&), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("const float& max( const float &in, const float &in )", asFUNCTIONPR(std::max<cgFloat>,(const cgFloat&, const cgFloat&), const cgFloat&), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float cos( float )", asFUNCTION(cosf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float sin( float )", asFUNCTION(sinf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float tan( float )", asFUNCTION(tanf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float acos( float )", asFUNCTION(acosf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float asin( float )", asFUNCTION(asinf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float atan( float )", asFUNCTION(atanf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float abs( float )", asFUNCTION(fabsf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float sqrt( float )", asFUNCTION(sqrtf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float exp( float )", asFUNCTION(expf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float pow( float, float )", asFUNCTION(powf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float ceil( float )", asFUNCTION(ceilf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float floor( float )", asFUNCTION(floorf), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float smooth( float, float, float )", asFUNCTION(cgMathUtility::smooth), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("int randomInt( int, int )", asFUNCTION(cgMathUtility::randomInt), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float randomFloat( float, float)", asFUNCTION(cgMathUtility::randomFloat), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float CGEToDegree( float )", asFUNCTION(toDegree), asCALL_CDECL) );
            BINDSUCCESS( engine->registerGlobalFunction("float CGEToRadian( float )", asFUNCTION(toRadian), asCALL_CDECL) );

            // Constants
            // ToDo: Add as defines to script pre-processor rather than using global properties at some point.
            static const cgDouble pi = CGE_PI;
            static const cgDouble two_pi = CGE_TWO_PI;
            static const cgDouble recip_pi = CGE_RECIP_PI;
            static const cgDouble epsilon_1m = CGE_EPSILON_1M;
            static const cgDouble epsilon_1cm = CGE_EPSILON_1CM;
            static const cgDouble epsilon_1mm = CGE_EPSILON_1MM;
            static const cgDouble epsilon_1um = CGE_EPSILON_1UM;
            static const cgDouble epsilon_1nm = CGE_EPSILON_1NM;
            static const cgDouble epsilon_single = CGE_EPSILON_SINGLE;
            static const cgDouble epsilon_double = CGE_EPSILON_DOUBLE;
            static const cgDouble epsilon = CGE_EPSILON;
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON", (void*)&epsilon ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON_1M", (void*)&epsilon_1m ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON_1CM", (void*)&epsilon_1cm ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON_1MM", (void*)&epsilon_1mm ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON_1UM", (void*)&epsilon_1um ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON_1NM", (void*)&epsilon_1nm ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON_SINGLE", (void*)&epsilon_single ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_EPSILON_DOUBLE", (void*)&epsilon_double ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_PI", (void*)&pi ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_TWO_PI", (void*)&two_pi ) );
            BINDSUCCESS( engine->registerGlobalProperty("const double CGE_RECIP_PI", (void*)&recip_pi ) );
        }

        static cgFloat toDegree( cgFloat x )
        {
            return CGEToDegree( x );
        }

        static cgFloat toRadian( cgFloat x) 
        {
            return CGEToRadian( x );
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Math