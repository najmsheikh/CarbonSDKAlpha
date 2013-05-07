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
// Name : cgScriptInterop.h                                                  //
//                                                                           //
// Desc : Simple namespace containing various different utility functions    //
//        and constants for providing interoperability with the scripting    //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGSCRIPTINTEROP_H_ )
#define _CGE_CGSCRIPTINTEROP_H_

//-----------------------------------------------------------------------------
// cgScriptInterop Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class asIObjectType;
class asIScriptEngine;

//-----------------------------------------------------------------------------
// Global Typedefs, Structures and Enumerations
//-----------------------------------------------------------------------------
// Describes all the different possible types of arguments
namespace cgScriptArgumentType
{
    enum Base
    {
        None = 0,
        Object,
        Address,
        Array,
        Byte,
        Bool,
        Word,
        DWord,
        QWord,
        Float,
        Double
    };

}; // End Enum : cgScriptArgumentType

typedef intptr_t cgScriptFunctionHandle;

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgScriptArgument (Class)
/// <summary>
/// An individual argument to be passed to a script during execution
/// of a specified function (i.e. via ExecuteScriptBool() ).
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgScriptArgument
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    CGE_VECTOR_DECLARE(cgScriptArgument, Array)

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    cgScriptArgument( ) : type(cgScriptArgumentType::Object), subType(cgScriptArgumentType::None), declaration(), data(0) {}
    cgScriptArgument( cgScriptArgumentType::Base _type, const cgString & _declaration, const void * _data )
        : type(_type), subType(cgScriptArgumentType::None), declaration(_declaration), data(_data )
    {
    } // End Constructor
    cgScriptArgument( cgScriptArgumentType::Base _type, cgScriptArgumentType::Base _subType, const cgString & _declaration, const void * _data )
        : type(_type), subType(_subType), declaration(_declaration), data(_data )
    {
    } // End Constructor
    cgScriptArgument( const cgVariant & v )
        : subType(cgScriptArgumentType::None), declaration( v.getTypeName() ), data( v.getData() )
    {
        switch ( v.getType() )
        {
            case cgVariant::Type_bool:
                type = cgScriptArgumentType::Bool;
                break;
            case cgVariant::Type_double:
                type = cgScriptArgumentType::Double;
                break;
            case cgVariant::Type_float:
                type = cgScriptArgumentType::Float;
                break;
            case cgVariant::Type_vector2:
                type = cgScriptArgumentType::Address;
                break;
            case cgVariant::Type_vector3:
                type = cgScriptArgumentType::Address;
                break;
            case cgVariant::Type_vector4:
                type = cgScriptArgumentType::Address;
                break;
            case cgVariant::Type_uint32:
            case cgVariant::Type_int32:
                type = cgScriptArgumentType::DWord;
                break;
            case cgVariant::Type_uint64:
            case cgVariant::Type_int64:
                type = cgScriptArgumentType::QWord;
                break;
            case cgVariant::Type_uint8:
            case cgVariant::Type_int8:
                type = cgScriptArgumentType::Byte;
                break;
            case cgVariant::Type_string:
                type = cgScriptArgumentType::Object;
                break;
        } // End switch

    } // End Constructor

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    cgScriptArgumentType::Base  type;           // Which type of argument is this? (i.e. object, dword etc)
    cgScriptArgumentType::Base  subType;        // Any subtype associated with this argument (template / array).
    cgString                    declaration;    // The declaration to use for this argument.
    const void                * data;           // A pointer to the actual argument.

}; // End Class cgScriptArgument

//-----------------------------------------------------------------------------
//  Name : cgScriptCompatibleStruct (Struct)
/// <summary>
/// A 'base' structure that provides a pure abstract virtual
/// for converting the structure to an argument list.
/// </summary>
//-----------------------------------------------------------------------------
struct CGE_API cgScriptCompatibleStruct
{
    virtual void  toArgumentList( cgScriptArgument::Array & arguments ) const = 0;
    
}; // End Struct cgScriptCompatibleStruct

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
        // Name : ExecuteException (Class)
        // Desc : Custom script execution exception object.
        //-----------------------------------------------------------------------------
        class CGE_API ExecuteException
        {
        public:
            //-------------------------------------------------------------------------
            // Constructors & Destructors
            //-------------------------------------------------------------------------
            ExecuteException( const cgChar * _description, const cgString & _scriptResource, cgInt _fileLine );

            //-------------------------------------------------------------------------
            // Public Methods
            //-------------------------------------------------------------------------
            cgString getExceptionSource() const;

            //-------------------------------------------------------------------------
            // Public Variables
            //-------------------------------------------------------------------------
            cgString description;
            cgString scriptResource;
            cgInt    fileLine;
        };

    }; // End Namespace : cgScriptInterop::Exceptions

    namespace Utils
    {
        //---------------------------------------------------------------------
        // Global Functions
        //---------------------------------------------------------------------
        // Retrieve the string representation of an angelscript error code.
        cgString CGE_API getResultName( cgInt resultCode );
    
    }; // End Namespace : cgScriptInterop::Utils

    namespace Types
    {
        //-----------------------------------------------------------------------------
        // Name : ScriptArray (Class)
        // Desc : Array class that allows for interaction between C++ and script code
        //        using templated array types (on the script side).
        //-----------------------------------------------------------------------------
        class CGE_API ScriptArray
        {
        public:
            //-------------------------------------------------------------------------
            // Constructors & Destructors
            //-------------------------------------------------------------------------
                     ScriptArray( cgUInt32 length, asIObjectType * objectType);
            virtual ~ScriptArray();

            //-------------------------------------------------------------------------
            // Public Methods
            //-------------------------------------------------------------------------
            void            addRef              ( );
            void            release             ( );

            // Type information
            asIObjectType * getArrayObjectType  ( ) const;
            int             getArrayTypeId      ( ) const;
            int             getElementTypeId    ( ) const;

            // Array methods
            void            resize              ( cgUInt32 elements );
            cgUInt32        getSize             ( );
            void          * at                  ( cgUInt32 index );

            //-------------------------------------------------------------------------
            // Public Static Methods
            //-------------------------------------------------------------------------
            static ScriptArray    * factory         ( asIObjectType * objectType );
            static ScriptArray    * factory         ( asIObjectType * objectType, cgUInt32 length );
            static ScriptArray    * listFactory     ( asIObjectType * objectType, cgUInt32 length );
            static void             bind            ( asIScriptEngine * engine );
            static bool             templateCallback(asIObjectType *ot, bool &dontGarbageCollect);

            //-------------------------------------------------------------------------
            // Public Operators
            //-------------------------------------------------------------------------
            ScriptArray & operator= ( const ScriptArray& );

        protected:
            //-------------------------------------------------------------------------
            // Protected Structures
            //-------------------------------------------------------------------------
            struct ArrayBuffer
            {
                cgUInt32 elementCount;
                cgByte   data[1];
            };

            //-------------------------------------------------------------------------
            // Protected Methods
            //-------------------------------------------------------------------------
            void            createBuffer        (ArrayBuffer **bufferVariable, cgUInt32 elements );
            void            deleteBuffer        (ArrayBuffer *buffer);
            void            copyBuffer          (ArrayBuffer *destination, ArrayBuffer *source);
            void            construct           (ArrayBuffer *buffer, cgUInt32 start, cgUInt32 end );
            void            destruct            (ArrayBuffer *buffer, cgUInt32 start, cgUInt32 end );
            bool            getGCFlag           ( );
            void            setGCFlag           ( );
            cgInt           getRefCount         ( );
            void            releaseAllHandles   ( asIScriptEngine * engine );
            void            enumReferences      ( asIScriptEngine *engine );

            //-------------------------------------------------------------------------
            // Protected Variables
            //-------------------------------------------------------------------------
            cgInt           mRefCount;
            cgInt           mSubTypeId;
            asIObjectType * mObjectType;
            ArrayBuffer   * mBuffer;
            bool            mIsArrayOfHandles;
            cgInt32         mElementSize;
            bool            mGCFlag;
        };

    }; // End Namespace : cgScriptInterop::Types

    //---------------------------------------------------------------------------------
    //  Name : DisposableScriptObject (Class)
    /// <summary>
    /// Base class from which objects should be derived if they need to be 
    /// passed to a script by reference as an object handle.
    /// </summary>
    //---------------------------------------------------------------------------------
    class CGE_API DisposableScriptObject
    {
    public:
        // Constructor
        DisposableScriptObject()
        {
            mScriptRefCount = 1;
        }

        // Requires virtual destructor.
        virtual ~DisposableScriptObject() { };

        // Add a reference to the internal counter.
        virtual void scriptAddRef( )
        {
            mScriptRefCount++;
        }

        // Remove a reference
        virtual void scriptReleaseRef( )
        {
            // Decrement reference count and destroy if applicable.
            mScriptRefCount--;
            if ( mScriptRefCount == 0 )
                delete this;
        }

        // Decrement reference count but don't actually delete when it hits zero.
        // Another interface will do this (multiple-inheritance fix)
        virtual void scriptDecrementRef( )
        {
            // Decrement reference count
            mScriptRefCount--;
        }

        // Dispose of managed data and remove a reference
        virtual void scriptSafeDispose( )
        {
            // Dispose of object data.
            dispose( true );

            // Decrement reference count and destroy if applicable.
            scriptReleaseRef();
        }

        //-----------------------------------------------------------------------------
        //  Name : dispose() (Virtual)
        /// <summary>
        /// Release any memory, references or resources allocated by this object. 
        /// Derived classes should implement this behavior.
        /// </summary>
        /// <remarks>
        /// This method is usually called automatically by the system either
        /// when the object is physically deleted, or when no references remain except
        /// those maintained by the scripting system. Calls can be made to this method 
        /// directly however if you wish the object to dispose of its data and clean 
        /// up prior to either of these circumstances occuring.
        ///
        /// <para>Manually calling this method can also be useful in cases where early
        /// disposal may be necessary in order to re-initialize the object's members 
        /// with alternate data; in effect simulating the process of object destruction
        /// without physically deleting the object and ensures that existing 
        /// references to the object remain valid.</para>
        /// 
        /// <para>In such situations the caller should take care to specify
        /// '<c>false</c>' to the '<paramref name="bDisposeBase" />' parameter.</para>
        /// </remarks>
        /// <param name="bDisposeBase">Call the base class' '<c>dispose()</c>' method 
        /// upon completion.</param>
        //-----------------------------------------------------------------------------
        virtual void dispose( bool disposeBase )
        {
            // Nothing in base implementation
        }

    protected:
        // Protected members
        int     mScriptRefCount;
        
    }; // End Class DisposableScriptObject

    //---------------------------------------------------------------------------------
    // Global Utility Macros
    //---------------------------------------------------------------------------------
    // Macros to assist with script object static functions and behavior registration.
    #define DECLARE_SCRIPTOBJECT(type,name) \
        public: \
        typedef type ScriptBaseType1; \
        typedef type ScriptBaseType2; \
        static cgUInt32 scriptGetBaseCount( ) \
        { \
            return 0; \
        } \
        static std::string scriptGetTypeName( ) \
        { \
            return name; \
        } \
        private:

    #define DECLARE_DERIVED_SCRIPTOBJECT(type,base,name) \
        public: \
        typedef base ScriptBaseType1; \
        typedef type ScriptBaseType2; \
        static cgUInt32 scriptGetBaseCount( ) \
        { \
            return 1; \
        } \
        static std::string scriptGetTypeName( ) \
        { \
            return name; \
        } \
        virtual void scriptAddRef( ) \
        { \
            base::scriptAddRef( ); \
        } \
        virtual void scriptReleaseRef( ) \
        { \
            base::scriptReleaseRef( ); \
        } \
        virtual void scriptDecrementRef( ) \
        { \
            base::scriptDecrementRef( ); \
        } \
        virtual void scriptSafeDispose( ) \
        { \
            dispose( true ); \
            scriptReleaseRef(); \
        } \
        private:
        
    #define DECLARE_DERIVED2_SCRIPTOBJECT(type,base1,base2,name) \
        public: \
        typedef base1 ScriptBaseType1; \
        typedef base2 ScriptBaseType2; \
        static cgUInt32 scriptGetBaseCount( ) \
        { \
            return 2; \
        } \
        static std::string scriptGetTypeName( ) \
        { \
            return name; \
        } \
        virtual void scriptAddRef( ) \
        { \
            base1::scriptAddRef( ); \
            base2::scriptAddRef( ); \
        } \
        virtual void scriptReleaseRef( ) \
        { \
            /* Only one should actually destroy on zero */ \
            base1::scriptDecrementRef( ); \
            base2::scriptReleaseRef( ); \
        } \
        virtual void scriptDecrementRef( ) \
        { \
            base1::scriptDecrementRef( ); \
            base2::scriptDecrementRef( ); \
        } \
        virtual void scriptSafeDispose( ) \
        { \
            dispose( true ); \
            scriptReleaseRef(); \
        } \
        private:

    // TODO: Remove
    /*#define DECLARE_SCRIPTOBJECT(type,name) \
        public: \
        static void registerScriptBehaviour( asIScriptEngine * internalEngine ) \
        { \
            BINDSUCCESS( internalEngine->RegisterObjectBehaviour( name, asBEHAVE_ADDREF, "void f()", asMETHOD(type,scriptAddRef), asCALL_THISCALL ) ); \
            BINDSUCCESS( internalEngine->RegisterObjectBehaviour( name, asBEHAVE_RELEASE, "void f()", asMETHOD(type,scriptSafeRelease), asCALL_THISCALL ) ); \
        } \
        void scriptAddRef( ) \
        { \
            mScriptRefCount++; \
        } \
        void scriptSafeRelease( ) \
        { \
            mScriptRefCount--; \
            if ( mScriptRefCount == 0 ) delete this; \
        } \
        static std::string scriptGetTypeName( ) \
        { \
            return name; \
        } \
        private: \
        int mScriptRefCount;

    #define DECLARE_SCRIPTOBJECT_CD(type,name) \
        public: \
        static void registerScriptBehaviour( asIScriptEngine * internalEngine ) \
        { \
            BINDSUCCESS( pInternalEngine->registerObjectBehaviour( name, asBEHAVE_ADDREF, "void f()", asMETHOD(type,scriptAddRef), asCALL_THISCALL ) ); \
            BINDSUCCESS( pInternalEngine->registerObjectBehaviour( name, asBEHAVE_RELEASE, "void f()", asMETHOD(type,scriptSafeRelease), asCALL_THISCALL ) ); \
            BINDSUCCESS( pInternalEngine->registerObjectBehaviour( name, asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(type::scriptConstruct), asCALL_CDECL_OBJLAST ) ); \
            BINDSUCCESS( pInternalEngine->registerObjectBehaviour( name, asBEHAVE_DESTRUCT, "void f()", asFUNCTION(type::scriptDestruct), asCALL_CDECL_OBJLAST ) ); \
        } \
        void scriptAddRef( ) \
        { \
            mScriptRefCount++; \
        } \
        void scriptSafeRelease( ) \
        { \
            mScriptRefCount--; \
            if ( mScriptRefCount == 0 ) delete this; \
        } \
        static void scriptConstruct( type *thisPointer ) \
        { \
            new(thisPointer) type(); \
        } \
        static void scriptDestruct( type *thisPointer ) \
        { \
            if ( thisPointer ) thisPointer->~type(); \
        } \
        static std::string scriptGetTypeName( ) \
        { \
            return name; \
        } \
        private: \
        int mScriptRefCount;

    #define DECLARE_DERIVED_SCRIPTOBJECT(type,base,name) \
        public: \
        static void registerScriptBehaviour( asIScriptEngine * internalEngine ) \
        { \
            BINDSUCCESS( internalEngine->registerObjectBehaviour( name, asBEHAVE_ADDREF, "void f()", asMETHOD(type,scriptAddRef), asCALL_THISCALL ) ); \
            BINDSUCCESS( internalEngine->registerObjectBehaviour( name, asBEHAVE_RELEASE, "void f()", asMETHOD(type,scriptSafeRelease), asCALL_THISCALL ) ); \
            BINDSUCCESS( internalEngine->registerObjectMethod( name, (base::ScriptGetTypeName() + "@+ Base()").c_str(), asMETHOD(type,scriptGetBase), asCALL_THISCALL ) ); \
        } \
        base * scriptGetBase( ) \
        { \
            return (base*)this; \
        } \
        static std::string scriptGetTypeName( ) \
        { \
            return name; \
        } \
        private: \

    #define DECLARE_DERIVED_SCRIPTOBJECT_NOBASE(type,base,name) \
        public: \
        static void registerScriptBehaviour( asIScriptEngine * internalEngine ) \
        { \
            BINDSUCCESS( internalEngine->registerObjectBehaviour( name, asBEHAVE_ADDREF, "void f()", asMETHOD(type,scriptAddRef), asCALL_THISCALL ) ); \
            BINDSUCCESS( internalEngine->registerObjectBehaviour( name, asBEHAVE_RELEASE, "void f()", asMETHOD(type,scriptSafeRelease), asCALL_THISCALL ) ); \
        } \
        static std::string scriptGetTypeName( ) \
        { \
            return name; \
        } \
        private: \

    #define INIT_SCRIPTOBJECT \
        mScriptRefCount = 1*/

}; // End Namespace : cgScriptInterop

#endif // !_CGE_CGSCRIPTINTEROP_H_