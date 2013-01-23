#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Physics/Controllers/cgRagdollController.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Physics { namespace Controllers {

// Package declaration
namespace RagdollController
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Physics.Controllers.RagdollController" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RagdollController", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgCharacterController (Class)
            ///////////////////////////////////////////////////////////////////////

            const cgChar * typeName = "RagdollController";

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgRagdollController>( engine );

            // Register base class object methods
            Core::Physics::Controllers::PhysicsController::registerControllerMethods<cgRagdollController>( engine, typeName );

            // Register the object behaviors
            BINDSUCCESS( engine->registerObjectBehavior( "RagdollController", asBEHAVE_FACTORY, "RagdollController @ f( PhysicsWorld@+ )", asFUNCTIONPR(ragdollControllerFactory, ( cgPhysicsWorld* ), cgRagdollController*), asCALL_CDECL) );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void restoreHierarchy( bool )", asMETHODPR(cgRagdollController, restoreHierarchy, ( bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyImpulseTo( ObjectNode@+, const Vector3 &in, const Vector3 &in )", asMETHODPR(cgRagdollController, applyImpulseTo, ( cgObjectNode*, const cgVector3&, const cgVector3& ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( typeName, "void applyImpulseTo( ObjectNode@+, const Vector3 &in )", asMETHODPR(cgRagdollController, applyImpulseTo, ( cgObjectNode*, const cgVector3& ), void ), asCALL_THISCALL) );

        }

        //---------------------------------------------------------------------
        //  Name : ragdollControllerFactory () (Static)
        /// <summary>
        /// Construct a new instance of the cgRagdollController class.
        /// </summary>
        //---------------------------------------------------------------------
        static cgRagdollController * ragdollControllerFactory( cgPhysicsWorld * world )
        {
            return new cgRagdollController( world );
        }

    }; // End Class : Package

} } } } } // End Namespace : cgScriptPackages::Core::Physics::Controllers::RagdollController