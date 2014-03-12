//---------------------------------------------------------------------------//
//              ____           _                         _                   //
//             / ___|__ _ _ __| |__   ___  _ __   __   _/ | __  __           //
//            | |   / _` | '__| '_ \ / _ \| '_ \  \ \ / / | \ \/ /           //
//            | |__| (_| | |  | |_) | (_) | | | |  \ V /| |_ >  <            //
//             \____\__,_|_|  |_.__/ \___/|_| |_|   \_/ |_(_)_/\_\           //
//                    Game Institute - Carbon Game Development Toolkit       //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// Name : cgBindingUtils.h                                                   //
//                                                                           //
// Desc : Simple namespace containing various different utility functions    //
//        that aid with binding application types and functionality to the   //
//        scripting system.                                                  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGBINDINGUTILS_H_ )
#define _CGE_CGBINDINGUTILS_H_

//-----------------------------------------------------------------------------
// cgBindingUtils Header Includes
//-----------------------------------------------------------------------------
#include "../../Lib/AngelScript/include/angelscript.h"
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Global Utility Macros
//-----------------------------------------------------------------------------
#define BINDSUCCESS(r) if(r<0)throw cgScriptInterop::Exceptions::BindException(r,cgDebugSource())

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgScriptEngine;

//-----------------------------------------------------------------------------
// cgScriptInterop Namespace
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScriptInterop (Namespace)
/// <summary>
/// Provides additional interoperability functions that bridge the gap
/// between the application and script engine.
/// </summary>
//-----------------------------------------------------------------------------
namespace cgScriptInterop
{
    namespace Exceptions
    {
        //-----------------------------------------------------------------------------
        // Name : BindException (Class)
        // Desc : Custom script binding exception object.
        //-----------------------------------------------------------------------------
        class CGE_API BindException
        {
        public:
            //-------------------------------------------------------------------------
            // Constructors & Destructors
            //-------------------------------------------------------------------------
            BindException( cgInt _error, const cgDebugSourceInfo & _debugSource );

            //-------------------------------------------------------------------------
            // Public Methods
            //-------------------------------------------------------------------------
            cgString getExceptionSource() const;

            //-------------------------------------------------------------------------
            // Public Variables
            //-------------------------------------------------------------------------
            cgInt               error;
            cgDebugSourceInfo   sourceFile;
        };

    }; // End Namespace : cgScriptInterop::Exceptions

    namespace Utils
    {
        //---------------------------------------------------------------------
        // Global Functions
        //---------------------------------------------------------------------
        // Cast a reference handle from one type to another.
        template<class _From, class _To>
        _To* referenceCast(_From* a)
        {
            // If the handle is already null handle, just return the null handle.
            if ( !a ) return 0;

            // Cast the pointer to the wanted type.
            _To * b = static_cast<_To*>(a);
            b->scriptAddRef();
            return b;
        
        } // End referenceCast
        template<class _From, class _Via1, class _To>
        _To* referenceCast2(_From* a)
        {
            // If the handle is already null handle, just return the null handle.
            if ( !a ) return 0;

            // Cast the pointer to the wanted type.
            _Via1 * v1 = static_cast<_Via1*>(a);
            _To * b = static_cast<_To*>(v1);
            b->scriptAddRef();
            return b;
        
        } // End referenceCast
        template<class _From, class _Via1, class _Via2, class _To>
        _To* referenceCast3(_From* a)
        {
            // If the handle is already null handle, just return the null handle.
            if ( !a ) return 0;

            // Cast the pointer to the wanted type.
            _Via1 * v1 = static_cast<_Via1*>(a);
            _Via2 * v2 = static_cast<_Via2*>(v1);
            _To * b = static_cast<_To*>(v2);
            b->scriptAddRef();
            return b;
        
        } // End referenceCast
        template<class _From, class _Via1, class _Via2, class _Via3, class _To>
        _To* referenceCast4(_From* a)
        {
            // If the handle is already null handle, just return the null handle.
            if ( !a ) return 0;

            // Cast the pointer to the wanted type.
            _Via1 * v1 = static_cast<_Via1*>(a);
            _Via2 * v2 = static_cast<_Via2*>(v1);
            _Via3 * v3 = static_cast<_Via3*>(v2);
            _To * b = static_cast<_To*>(v3);
            b->scriptAddRef();
            return b;
        
        } // End referenceCast
        template<class _From, class _Via1, class _Via2, class _Via3, class _Via4, class _To>
        _To* referenceCast5(_From* a)
        {
            // If the handle is already null handle, just return the null handle.
            if ( !a ) return 0;

            // Cast the pointer to the wanted type.
            _Via1 * v1 = static_cast<_Via1*>(a);
            _Via2 * v2 = static_cast<_Via2*>(v1);
            _Via3 * v3 = static_cast<_Via3*>(v2);
            _Via4 * v4 = static_cast<_Via4*>(v3);
            _To * b = static_cast<_To*>(v4);
            b->scriptAddRef();
            return b;
        
        } // End referenceCast
        template<class _From, class _Via1, class _Via2, class _Via3, class _Via4, class _Via5, class _To>
        _To* referenceCast6(_From* a)
        {
            // If the handle is already null handle, just return the null handle.
            if ( !a ) return 0;

            // Cast the pointer to the wanted type.
            _Via1 * v1 = static_cast<_Via1*>(a);
            _Via2 * v2 = static_cast<_Via2*>(v1);
            _Via3 * v3 = static_cast<_Via3*>(v2);
            _Via4 * v4 = static_cast<_Via4*>(v3);
            _Via5 * v5 = static_cast<_Via5*>(v4);
            _To * b = static_cast<_To*>(v5);
            b->scriptAddRef();
            return b;
        
        } // End referenceCast

        // Register optional cast behaviors for types intended to be used as object handles. This is
        // a recursive procedure that allows casts to occur between all classes in the hierarchy.
        template<class T, class TBase >
        void registerHandleCasts( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_IMPLICIT_REF_CAST, (TBase::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast<T,TBase>)), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( TBase::scriptGetTypeName().c_str(), asBEHAVE_REF_CAST, (T::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast<TBase,T>)), asCALL_CDECL_OBJLAST) );

            // Recurse to catch base types.
            if ( TBase::scriptGetBaseCount() >= 1 )
                registerHandleCasts2<T, TBase, TBase::ScriptBaseType1>( engine );
            if ( TBase::scriptGetBaseCount() >= 2 )
                registerHandleCasts2<T, TBase, TBase::ScriptBaseType2>( engine );
        
        } // End registerHandleCasts
        template<class T, class TVia1, class TBase >
        void registerHandleCasts2( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_IMPLICIT_REF_CAST, (TBase::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast2<T,TVia1,TBase>)), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( TBase::scriptGetTypeName().c_str(), asBEHAVE_REF_CAST, (T::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast2<TBase,TVia1,T>)), asCALL_CDECL_OBJLAST) );

            // Recurse to catch base types.
            if ( TBase::scriptGetBaseCount() >= 1 )
                registerHandleCasts3<T, TVia1, TBase, TBase::ScriptBaseType1>( engine );
            if ( TBase::scriptGetBaseCount() >= 2 )
                registerHandleCasts3<T, TVia1, TBase, TBase::ScriptBaseType2>( engine );
        
        } // End registerHandleCasts2
        template<class T, class TVia1, class TVia2, class TBase >
        void registerHandleCasts3( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_IMPLICIT_REF_CAST, (TBase::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast3<T,TVia1,TVia2,TBase>)), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( TBase::scriptGetTypeName().c_str(), asBEHAVE_REF_CAST, (T::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast3<TBase,TVia2,TVia1,T>)), asCALL_CDECL_OBJLAST) );

            // Recurse to catch base types.
            if ( TBase::scriptGetBaseCount() >= 1 )
                registerHandleCasts4<T, TVia1, TVia2, TBase, TBase::ScriptBaseType1>( engine );
            if ( TBase::scriptGetBaseCount() >= 2 )
                registerHandleCasts4<T, TVia1, TVia2, TBase, TBase::ScriptBaseType2>( engine );
        
        } // End registerHandleCasts3
        template<class T, class TVia1, class TVia2, class TVia3, class TBase >
        void registerHandleCasts4( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_IMPLICIT_REF_CAST, (TBase::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast4<T,TVia1,TVia2,TVia3,TBase>)), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( TBase::scriptGetTypeName().c_str(), asBEHAVE_REF_CAST, (T::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast4<TBase,TVia3,TVia2,TVia1,T>)), asCALL_CDECL_OBJLAST) );

            // Recurse to catch base types.
            if ( TBase::scriptGetBaseCount() >= 1 )
                registerHandleCasts5<T, TVia1, TVia2, TVia3, TBase, TBase::ScriptBaseType1>( engine );
            if ( TBase::scriptGetBaseCount() >= 2 )
                registerHandleCasts5<T, TVia1, TVia2, TVia3, TBase, TBase::ScriptBaseType2>( engine );
            
        } // End registerHandleCasts4
        template<class T, class TVia1, class TVia2, class TVia3, class TVia4, class TBase >
        void registerHandleCasts5( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_IMPLICIT_REF_CAST, (TBase::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast5<T,TVia1,TVia2,TVia3,TVia4,TBase>)), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( TBase::scriptGetTypeName().c_str(), asBEHAVE_REF_CAST, (T::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast5<TBase,TVia4,TVia3,TVia2,TVia1,T>)), asCALL_CDECL_OBJLAST) );

            // Recurse to catch base types.
            if ( TBase::scriptGetBaseCount() >= 1 )
                registerHandleCasts6<T, TVia1, TVia2, TVia3, TVia4, TBase, TBase::ScriptBaseType1>( engine );
            if ( TBase::scriptGetBaseCount() >= 2 )
                registerHandleCasts6<T, TVia1, TVia2, TVia3, TVia4, TBase, TBase::ScriptBaseType2>( engine );
        
        } // End registerHandleCasts5
        template<class T, class TVia1, class TVia2, class TVia3, class TVia4, class TVia5, class TBase >
        void registerHandleCasts6( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_IMPLICIT_REF_CAST, (TBase::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast6<T,TVia1,TVia2,TVia3,TVia4,TVia5,TBase>)), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( TBase::scriptGetTypeName().c_str(), asBEHAVE_REF_CAST, (T::scriptGetTypeName() + "@ f()").c_str(), asFUNCTION((cgScriptInterop::Utils::referenceCast6<TBase,TVia5,TVia4,TVia3,TVia2,TVia1,T>)), asCALL_CDECL_OBJLAST) );

            // Recurse to catch base types.
            if ( TBase::scriptGetBaseCount() >= 1 )
            {
                cgAssertEx( false, "Nested script reference cast too deep. Consider simplifying the class hierarchy." );
            }
            if ( TBase::scriptGetBaseCount() >= 2 )
            {
                cgAssertEx( false, "Nested script reference cast too deep. Consider simplifying the class hierarchy." );
            }
        
        } // End registerHandleCasts6

        // Register standard ScriptAddRef() / ScriptRelease() and optional implicit cast
        // behaviors for types intended to be used as object handles.
        template<class T>
        void registerHandleBehaviors( cgScriptEngine * engine )
        {
            // Add standard AddRef / Release() behaviors.
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_ADDREF, "void f()", asMETHOD(T,scriptAddRef), asCALL_THISCALL ) );
            BINDSUCCESS( engine->registerObjectBehavior( T::scriptGetTypeName().c_str(), asBEHAVE_RELEASE, "void f()", asMETHOD(T,scriptReleaseRef), asCALL_THISCALL ) );
            
            // Add casts for base types.
            if ( T::scriptGetBaseCount() >= 1 )
                registerHandleCasts<T,T::ScriptBaseType1>( engine );
            if ( T::scriptGetBaseCount() >= 2 )
                registerHandleCasts<T,T::ScriptBaseType2>( engine );
        
        } // End registerHandleBehaviors

        // Register basic, default constructor, destructor and assignment behaviors.
        template<class T>
        void registerDefaultCDA( cgScriptEngine * pEngine, const cgChar * lpszName )
        {
            std::string sName = lpszName;
            BINDSUCCESS( pEngine->registerObjectBehavior( lpszName, asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(defaultConstructor<T>,(T*), void), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( pEngine->registerObjectBehavior( lpszName, asBEHAVE_DESTRUCT, "void f()", asFUNCTIONPR(defaultDestructor<T>,(T*), void), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( pEngine->registerObjectMethod( lpszName, (sName + " &opAssign(const " + sName + " &in)").c_str(), asFUNCTIONPR(defaultAssignment<T>,(const T&,T*), T&), asCALL_CDECL_OBJLAST) );
            
        } // End registerDefaultCDA

        template<class T>
        void defaultConstructor( T* thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) T();
        }

        template<class T>
        void defaultDestructor( T* thisPointer )
        {
            thisPointer->~T();
        }

        template<class T>
        T& defaultAssignment( const T & rhs, T* thisPointer )
        {
            *thisPointer = rhs;
            return *thisPointer;
        }

        //-----------------------------------------------------------------------------
        // Name : ArrayBindHelperBase (Template Class)
        // Desc : Base class providing registration helper functionality for
        //        cgArray<> types.
        //-----------------------------------------------------------------------------
        template< class _elemType >
        class ArrayBindHelperBase
        {
        public:
            //-------------------------------------------------------------------------
            // Public Static Template Methods
            //-------------------------------------------------------------------------
            //-------------------------------------------------------------------------
            // Name : registerType() (Static)
            // Desc : Register an engine type that provides support for a specialized
            //        array with the necessary type / behavior.
            //-------------------------------------------------------------------------
            static void registerType( cgScriptEngine * engine, const cgChar * typeName, const cgChar * subTypeName )
            {
                BINDSUCCESS( engine->registerObjectType( typeName, sizeof(cgArray<_elemType>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            }

            //-------------------------------------------------------------------------
            //  Name : construct() (Static)
            /// <summary>
            /// This is a wrapper for the cgArray type constructor.
            /// </summary>
            //-------------------------------------------------------------------------
            static void construct( cgArray<_elemType> * thisPointer )
            {
                // Use placement new to allocate which will in turn call the constructor
                new(thisPointer) cgArray<_elemType>();
            }

            //-------------------------------------------------------------------------
            //  Name : construct() (Static)
            /// <summary>
            /// This is a wrapper for the cgArray type constructor.
            /// </summary>
            //-------------------------------------------------------------------------
            static void construct( const cgArray<_elemType> & srcVec, cgArray<_elemType> * thisPointer )
            {
                // Use placement new to allocate which will in turn call the constructor
                new(thisPointer) cgArray<_elemType>(srcVec);
            }

            //-------------------------------------------------------------------------
            //  Name : construct() (Static)
            /// <summary>
            /// This is a wrapper for the cgArray type constructor.
            /// </summary>
            //-------------------------------------------------------------------------
            static void construct( cgUInt32 elements, cgArray<_elemType> * thisPointer )
            {
                // Use placement new to allocate which will in turn call the constructor
                new(thisPointer) cgArray<_elemType>(elements);
            }

            //-------------------------------------------------------------------------
            //  Name : destruct() (Static)
            /// <summary>
            /// A wrapper for the cgArray type destructor. This will be triggered
            /// whenever it goes out of scope inside the script.
            /// </summary>
            //-------------------------------------------------------------------------
            static void destruct( cgArray<_elemType> * thisPointer )
            {
                thisPointer->~cgArray();
            }

            //-------------------------------------------------------------------------
            //  Name : listConstruct() (Static)
            /// <summary>
            /// A wrapper for the cgArray type constructor that supports
            /// initialization lists.
            /// </summary>
            //-------------------------------------------------------------------------
            static void listConstruct( void * initList, cgArray<_elemType> * thisPointer )
            {
                asUINT elements = *(asUINT*)initList;
                _elemType * data = (_elemType*)(((cgByte*)initList) + sizeof(asUINT));

                // Allocate the vector
                cgArray<_elemType> * internalArray = thisPointer;
                new(internalArray) cgArray<_elemType>(elements);

                // Copy data.
                for ( asUINT i = 0; i < elements; ++i )
                    (*internalArray)[i] = data[i];
            }

            //-------------------------------------------------------------------------
            //  Name : assign() (Static)
            /// <summary>
            /// Duplicate one vector of this type into another.
            /// </summary>
            //-------------------------------------------------------------------------
            static cgArray<_elemType>& assign( const cgArray<_elemType> & srcVec, cgArray<_elemType> * thisPointer )
            {
                *thisPointer = srcVec;
                return *thisPointer;
            }

            //-------------------------------------------------------------------------
            //  Name : size() (Static)
            /// <summary>
            /// Retrieve the size of the vector.
            /// </summary>
            //-------------------------------------------------------------------------
            static cgUInt32 size( cgArray<_elemType> * thisPointer )
            {
                return (cgUInt32)thisPointer->size();
            }

            //-------------------------------------------------------------------------
            //  Name : resize() (Static)
            /// <summary>
            /// Adjust the size of the vector.
            /// </summary>
            //-------------------------------------------------------------------------
            static void resize( cgUInt32 elements, cgArray<_elemType> * thisPointer )
            {
                thisPointer->resize( elements );
            }

            //-------------------------------------------------------------------------
            //  Name : reserve() (Static)
            /// <summary>
            /// Adjust the internal capacity of the vector.
            /// </summary>
            //-------------------------------------------------------------------------
            static void reserve( cgUInt32 elements, cgArray<_elemType> * thisPointer )
            {
                thisPointer->reserve( elements );
            }

            //-------------------------------------------------------------------------
            //  Name : insertAt() (Static)
            /// <summary>
            /// Insert the specified value at the indicated location.
            /// </summary>
            //-------------------------------------------------------------------------
            static void insertAt( cgUInt32 index, _elemType * value, cgArray<_elemType> * thisPointer )
            {
                thisPointer->insert( ( index >= thisPointer->size() ) ? thisPointer->end() : thisPointer->begin() + index, *value );
            }

            //-------------------------------------------------------------------------
            //  Name : insertLast() (Static)
            /// <summary>
            /// Insert the specified value at the end of the array.
            /// </summary>
            //-------------------------------------------------------------------------
            static void insertLast( _elemType * value, cgArray<_elemType> * thisPointer )
            {
                thisPointer->push_back( *value );
            }

            //-------------------------------------------------------------------------
            //  Name : removeAt() (Static)
            /// <summary>
            /// Remove the specified element from the array.
            /// </summary>
            //-------------------------------------------------------------------------
            static void removeAt( cgUInt32 index, cgArray<_elemType> * thisPointer )
            {
                if ( !thisPointer->empty() )
                    thisPointer->erase( ( index >= thisPointer->size() ) ? thisPointer->end() : thisPointer->begin() + index );
            }

            //-------------------------------------------------------------------------
            //  Name : removeLast() (Static)
            /// <summary>
            /// Remove the final element in the array.
            /// </summary>
            //-------------------------------------------------------------------------
            static void removeLast( cgArray<_elemType> * thisPointer )
            {
                thisPointer->pop_back();
            }

            //-------------------------------------------------------------------------
            //  Name : isEmpty() (Static)
            /// <summary>
            /// Determine if the specified array is empty.
            /// </summary>
            //-------------------------------------------------------------------------
            static bool isEmpty( cgArray<_elemType> * thisPointer )
            {
                return thisPointer->empty();
            }
        };

        //-----------------------------------------------------------------------------
        // Name : ArrayBindHelper (Template Class)
        // Desc : Class providing registration helper functionality for cgArray<> 
        //        types. The '_nonReferencable' parameter (defaults to false) should
        //        be set to true if element accessor methods cannot be returned by
        //        reference. An example of this might be a specialized container 
        //        implementation similar to the standard std::vector<bool> type that
        //        uses a bitfield to compact storage. A specialization exists for this 
        //        case (see following class).
        //-----------------------------------------------------------------------------
        template< class _elemType, bool _nonReferenceable = false >
        class ArrayBindHelper : public ArrayBindHelperBase<_elemType>
        {
        public:
            //-------------------------------------------------------------------------
            // Public Static Template Methods
            //-------------------------------------------------------------------------
            //-------------------------------------------------------------------------
            // Name : registerMethods() (Static)
            // Desc : Register the methods necessary to provide a specialized
            //        cgArray type.
            //-------------------------------------------------------------------------
            static void registerMethods( cgScriptEngine * engine, const cgChar * typeName, const cgChar * subTypeName )
            {
                std::string sName = typeName;
                std::string sSubType = subTypeName;

                // Register the object behaviors
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(construct,(cgArray<_elemType>*), void),  asCALL_CDECL_OBJLAST ) );
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_DESTRUCT, "void f()", asFUNCTIONPR(destruct,(cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST ) );
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, ("void f(const "+sName+" &in)").c_str(), asFUNCTIONPR(construct,(const cgArray<_elemType>&,cgArray<_elemType>*),void), asCALL_CDECL_OBJLAST ) );
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, "void f(uint)", asFUNCTIONPR(construct,(cgUInt32,cgArray<_elemType>*),void), asCALL_CDECL_OBJLAST ) );
                
                // Register the constructor that will be used for initialization lists
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_LIST_CONSTRUCT, ("void f(int&in) {repeat " + sSubType + "}").c_str(), asFUNCTIONPR(listConstruct, (void*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );

                // Register the object operators
                BINDSUCCESS( engine->registerObjectMethod( typeName, (sSubType + " &opIndex(uint)").c_str(), asFUNCTIONPR(index,(cgUInt32,cgArray<_elemType>*), _elemType*), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("const " + sSubType + " &opIndex(uint) const").c_str(), asFUNCTIONPR(index,(cgUInt32,cgArray<_elemType>*), _elemType*), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, (sName + " &opAssign(const " + sName + " &in)").c_str(), asFUNCTIONPR(assign,(const cgArray<_elemType>&,cgArray<_elemType>*), cgArray<_elemType>&), asCALL_CDECL_OBJLAST) );

                // Other methods
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void insertAt(uint, const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertAt,(cgUInt32, _elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void insertLast(const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertLast,(_elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void removeAt(uint)", asFUNCTIONPR(removeAt,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void removeLast()", asFUNCTIONPR(removeLast,(cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "uint length() const", asFUNCTIONPR(size,(cgArray<_elemType>*), cgUInt32), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void reserve(uint)", asFUNCTIONPR(reserve,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void resize(uint)", asFUNCTIONPR(resize,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isEmpty() const", asFUNCTIONPR(isEmpty,(cgArray<_elemType>*), bool), asCALL_CDECL_OBJLAST) );

                // Virtual Properties
                BINDSUCCESS( engine->registerObjectMethod( typeName, "uint get_length() const", asFUNCTIONPR(size,(cgArray<_elemType>*), cgUInt32), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_length(uint)", asFUNCTIONPR(resize,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );

                // Register STL compatible method name aliases.
                BINDSUCCESS( engine->registerObjectMethod( typeName, "uint size() const", asFUNCTIONPR(size,(cgArray<_elemType>*), cgUInt32), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "bool empty() const", asFUNCTIONPR(isEmpty,(cgArray<_elemType>*), bool), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void push_back(const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertLast,(_elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void pop_back()", asFUNCTIONPR(removeLast,(cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void insert(uint, const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertAt,(cgUInt32, _elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void erase(uint)", asFUNCTIONPR(removeAt,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
            
            } // End Method registerMethods<>

            //-------------------------------------------------------------------------
            //  Name : index() (Static)
            /// <summary>
            /// A wrapper around the cgArray indexing operator that provides the
            /// bounds checking required by AS specification.
            /// </summary>
            //-------------------------------------------------------------------------
            static _elemType* index( cgUInt32 element, cgArray<_elemType> * thisPointer )
            {
                if ( element < 0 || element >= thisPointer->size())
                {
	                asIScriptContext * context = asGetActiveContext();
	                if ( context )
		                context->SetException("Array index out of bounds.");
	                return 0;
                }
                return &(*thisPointer)[element];
            }
        };

        //-----------------------------------------------------------------------------
        // Name : ArrayBindHelper (Template Class)
        // Desc : Class providing registration helper functionality for cgArray<> 
        //        types. Specialization for cases where '_nonReferencable' template 
        //        parameter was set to 'true'.
        //-----------------------------------------------------------------------------
        template< class _elemType >
        class ArrayBindHelper<_elemType,true> : public ArrayBindHelperBase<_elemType>
        {
        public:
            //-------------------------------------------------------------------------
            // Public Static Template Methods
            //-------------------------------------------------------------------------
            //-------------------------------------------------------------------------
            // Name : registerMethods() (Static)
            // Desc : Register the methods necessary to provide a specialized
            //        cgArray type.
            //-------------------------------------------------------------------------
            static void registerMethods( cgScriptEngine * engine, const cgChar * typeName, const cgChar * subTypeName )
            {
                std::string sName = typeName;
                std::string sSubType = subTypeName;

                // Register the object behaviors
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(construct,(cgArray<_elemType>*), void),  asCALL_CDECL_OBJLAST ) );
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_DESTRUCT, "void f()", asFUNCTIONPR(destruct,(cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST ) );
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, ("void f(const "+sName+" &in)").c_str(), asFUNCTIONPR(construct,(const cgArray<_elemType>&,cgArray<_elemType>*),void), asCALL_CDECL_OBJLAST ) );
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, "void f(uint)", asFUNCTIONPR(construct,(cgUInt32,cgArray<_elemType>*),void), asCALL_CDECL_OBJLAST ) );

                // Register the constructor that will be used for initialization lists
                BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_LIST_CONSTRUCT, ("void f(int&in) {repeat " + sSubType + "}").c_str(), asFUNCTIONPR(listConstruct, (void*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );

                // In this specialization, the vector does not allow references to be taken
                // and thus uses by-value returns instead of a reference.
                BINDSUCCESS( engine->registerObjectMethod( typeName, (sSubType + " opIndex(uint)").c_str(), asFUNCTIONPR(index,(cgUInt32,cgArray<_elemType>*), _elemType), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, (sName + " opAssign(const " + sName + " &in)").c_str(), asFUNCTIONPR(assign,(const cgArray<_elemType>&,cgArray<_elemType>*), cgArray<_elemType>&), asCALL_CDECL_OBJLAST) );

                // Other methods
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void insertAt(uint, const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertAt,(cgUInt32, _elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void insertLast(const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertLast,(_elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void removeAt(uint)", asFUNCTIONPR(removeAt,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void removeLast()", asFUNCTIONPR(removeLast,(cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "uint length() const", asFUNCTIONPR(size,(cgArray<_elemType>*), cgUInt32), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void reserve(uint)", asFUNCTIONPR(reserve,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void resize(uint)", asFUNCTIONPR(resize,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isEmpty() const", asFUNCTIONPR(isEmpty,(cgArray<_elemType>*), bool), asCALL_CDECL_OBJLAST) );

                // Virtual Properties
                BINDSUCCESS( engine->registerObjectMethod( typeName, "uint get_length() const", asFUNCTIONPR(size,(cgArray<_elemType>*), cgUInt32), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void set_length(uint)", asFUNCTIONPR(resize,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );

                // Register STL compatible method name aliases.
                BINDSUCCESS( engine->registerObjectMethod( typeName, "uint size() const", asFUNCTIONPR(size,(cgArray<_elemType>*), cgUInt32), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "bool empty() const", asFUNCTIONPR(isEmpty,(cgArray<_elemType>*), bool), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void push_back(const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertLast,(_elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void pop_back()", asFUNCTIONPR(removeLast,(cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, ("void insert(uint, const " + sSubType + "&in)").c_str(), asFUNCTIONPR(insertAt,(cgUInt32, _elemType*, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
                BINDSUCCESS( engine->registerObjectMethod( typeName, "void erase(uint)", asFUNCTIONPR(removeAt,(cgUInt32, cgArray<_elemType>*), void), asCALL_CDECL_OBJLAST) );
            
            } // End Method registerMethods<>

            //-------------------------------------------------------------------------
            //  Name : index() (Static)
            /// <summary>
            /// A wrapper around the cgArray indexing operator that provides the
            /// bounds checking required by AS specification. Unlike the regular 
            /// 'index' function, this specialization returns a /copy/ of the 
            /// underlying element in cases where that value is non-referencable 
            /// (i.e. std::vector<bool> like containers)
            /// </summary>
            //-------------------------------------------------------------------------
            static _elemType index( cgUInt32 element, cgArray<_elemType> * thisPointer )
            {
                if ( element < 0 || element >= thisPointer->size())
                {
	                asIScriptContext * context = asGetActiveContext();
	                if ( context )
		                context->SetException("Array index out of bounds.");
	                return 0;
                }
                return (*thisPointer)[element];
            }
        };
    
    }; // End Namespace : cgScriptInterop::Utils

}; // End Namespace : cgScriptInterop

#endif // !_CGE_CGBINDINGUTILS_H_