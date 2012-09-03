#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgScript.h>

// Child type packages
#include "System/String.h"
#include "System/UID.h"
#include "System/Rect.h"
#include "System/RectF.h"
#include "System/Point.h"
#include "System/PointF.h"
#include "System/Range.h"
#include "System/RangeF.h"
#include "System/Size.h"
#include "System/SizeF.h"
#include "System/DebugSource.h"
#include "System/Variant.h"
#include "System/PropertyContainer.h"

// Child container packages
#include "System/References.h"
#include "System/Events.h"
#include "System/IO.h"
#include "System/Utilities.h"

// Parent hierarchy
namespace cgScriptPackages { namespace Core {

// Package declaration
namespace System
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System" )
            
            // Type packages
            DECLARE_PACKAGE_CHILD( String )
            DECLARE_PACKAGE_CHILD( DebugSource )
            DECLARE_PACKAGE_CHILD( UID )
            DECLARE_PACKAGE_CHILD( Rect )
            DECLARE_PACKAGE_CHILD( RectF )
            DECLARE_PACKAGE_CHILD( Point )
            DECLARE_PACKAGE_CHILD( PointF )
            DECLARE_PACKAGE_CHILD( Range )
            DECLARE_PACKAGE_CHILD( RangeF )
            DECLARE_PACKAGE_CHILD( Size )
            DECLARE_PACKAGE_CHILD( SizeF )
            DECLARE_PACKAGE_CHILD( Variant )
            DECLARE_PACKAGE_CHILD( PropertyContainer )
            
            // Container packages
            DECLARE_PACKAGE_CHILD( References )
            DECLARE_PACKAGE_CHILD( Events )
            DECLARE_PACKAGE_CHILD( IO )
            DECLARE_PACKAGE_CHILD( Utilities )

        END_SCRIPT_PACKAGE( )

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            ///////////////////////////////////////////////////////////////////////
            // System Utilities
            ///////////////////////////////////////////////////////////////////////
            // Register standard engine utilities (such as __FILE__ / __LINE__ analogs)
            engine->registerGlobalFunction( "String executingFile()", asFUNCTION(executingFile), asCALL_CDECL );
            engine->registerGlobalFunction( "uint executingLine()", asFUNCTION(executingLine), asCALL_CDECL );
        }

        //---------------------------------------------------------------------
        //  Name : executingFile ()
        /// <summary>
        /// Utility function to retrieve the currently executing script name.
        /// </summary>
        //---------------------------------------------------------------------
        static cgString executingFile()
        {
            cgScript * script = (cgScript*)asGetActiveContext()->GetUserData();
            if ( script != CG_NULL )
                return script->getResourceName();
            return cgString::Empty;;
        }

        //---------------------------------------------------------------------
        //  Name : executingLine ()
        /// <summary>
        /// Utility function to retrieve the currently executing script line.
        /// </summary>
        //---------------------------------------------------------------------
        static cgUInt32 executingLine()
        {
            return asGetActiveContext()->GetLineNumber();
        }

    }; // End Class : Package

} } } // End Namespace : cgScriptPackages::Core::System