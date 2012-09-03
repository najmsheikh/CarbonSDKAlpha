#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>

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
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::Math