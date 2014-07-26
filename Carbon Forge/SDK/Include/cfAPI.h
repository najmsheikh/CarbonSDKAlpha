//---------------------------------------------------------------------------//
//         ____           _                   _____                          //
//        / ___|__ _ _ __| |__   ___  _ __   |  ___|__  _ __ __ _  ___       //
//       | |   / _` | '__| '_ \ / _ \| '_ \  | |_ / _ \| '__/ _` |/ _ \      //
//       | |__| (_| | |  | |_) | (_) | | | | |  _| (_) | | | (_| |  __/      //
//        \____\__,_|_|  |_.__/ \___/|_| |_| |_|  \___/|_|  \__, |\___|      //
//                   Game Institute - Carbon Engine Sandbox |___/            //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// File : cfAPI.h                                                            //
//                                                                           //
// Desc : Provides support types, macros and other items necessary to        //
//        interoperate correctly between managed and native systems & types. //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// Global Macros
//-----------------------------------------------------------------------------
#if defined( CARBONFORGE_DLL_EXPORTS )
#   define CARBONFORGE_API __declspec( dllexport )
#elif defined( CARBONFORGE_DLL_IMPORTS )
#   define CARBONFORGE_API __declspec( dllimport )
#else
#   define CARBONFORGE_API
#endif

//-----------------------------------------------------------------------------
// Global Forward Declarations
//-----------------------------------------------------------------------------
class cgTexture;

// Support for managed System::String conversion
#include <vcclr.h>
#include <cgBaseTypes.h>
namespace CarbonForge
{
    #ifdef __cplusplus_cli
    #define __GCHANDLE_TO_VOIDPTR(x) ((GCHandle::operator System::IntPtr(x)).ToPointer())
    #define __VOIDPTR_TO_GCHANDLE(x) (GCHandle::operator GCHandle(System::IntPtr(x)))
    #define __NULLPTR nullptr
    #else
    #define __GCHANDLE_TO_VOIDPTR(x) ((GCHandle::op_Explicit(x)).ToPointer())
    #define __VOIDPTR_TO_GCHANDLE(x) (GCHandle::op_Explicit(x))
    #define __NULLPTR 0
    #endif

    //-------------------------------------------------------------------------
    // Name : gcrootx (Template Class)
    /// <summary>
    /// Our specific implementation of 'gcroot' that adds additional
    /// functionality specific to our application. Such features include
    /// an overloaded operator==, etc.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class T> class gcrootx
    {
    public:
        //---------------------------------------------------------------------
        // Public Typedefs, Structures & Enumerations
        //---------------------------------------------------------------------
	    typedef System::Runtime::InteropServices::GCHandle GCHandle;

        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        // Name : gcrootx () (Constructor)
        // Desc : Default constructor. Initializes internal handle with nullptr
        //---------------------------------------------------------------------
        gcrootx() : _handle( __GCHANDLE_TO_VOIDPTR(GCHandle::Alloc(__NULLPTR)) ) {}
        
        //---------------------------------------------------------------------
        // Name : gcrootx () (Constructor)
        // Desc : Initializing constructor. Initializes internal handle with 
        //        a copy of the specified handle value.
        //---------------------------------------------------------------------
        gcrootx(T t) : _handle( __GCHANDLE_TO_VOIDPTR(GCHandle::Alloc(t)) ) {}

        //---------------------------------------------------------------------
        // Name : gcrootx () (Copy Constructor)
        // Desc : Copy constructor.
        //---------------------------------------------------------------------
        gcrootx(const gcrootx& r) : _handle( __GCHANDLE_TO_VOIDPTR( GCHandle::Alloc( __VOIDPTR_TO_GCHANDLE(r._handle).Target )) ) {}

        //---------------------------------------------------------------------
        // Name : ~gcrootx () (Destructor)
        // Desc : Class Destructor.
        //---------------------------------------------------------------------
        ~gcrootx()
        {
		    GCHandle g = __VOIDPTR_TO_GCHANDLE(_handle);
		    g.Free();
		    _handle = 0; // should fail if reconstituted
	    }

        //---------------------------------------------------------------------
        // Public Operators
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        // Name : operator= (T)
        // Desc : Assignment operator.
        //---------------------------------------------------------------------
	    gcrootx & operator=(T t)
        {
		    __VOIDPTR_TO_GCHANDLE(_handle).Target = t;
		    return *this;
	    }

        //---------------------------------------------------------------------
        // Name : operator= (gcrootx)
        // Desc : Assignment operator.
        //---------------------------------------------------------------------
	    gcrootx & operator=( const gcrootx & r )
        {
		    T t = (T)r;
		    __VOIDPTR_TO_GCHANDLE(_handle).Target = t;
		    return *this;
	    }

        //---------------------------------------------------------------------
        // Name : operator T()
        // Desc : Cast operator.
        //---------------------------------------------------------------------
	    operator T () const
        {
		    return static_cast<T>( __VOIDPTR_TO_GCHANDLE(_handle).Target );
	    }

        //---------------------------------------------------------------------
        // Name : operator*()
        // Desc : Dereference operator.
        //---------------------------------------------------------------------
	    T operator*() const
        {
		    return static_cast<T>(__VOIDPTR_TO_GCHANDLE(_handle).Target);
	    }

        //---------------------------------------------------------------------
        // Name : operator ->()
        // Desc : Dereference operator.
        //---------------------------------------------------------------------
	    T operator->() const
        {
		    return static_cast<T>(__VOIDPTR_TO_GCHANDLE(_handle).Target);
	    }

        //---------------------------------------------------------------------
        // Name : operator==()
        // Desc : Comparison operator
        //---------------------------------------------------------------------
        bool operator==( T t )
        {
            return (static_cast<T>(__VOIDPTR_TO_GCHANDLE(_handle).Target) == t);
        }

        //---------------------------------------------------------------------
        // Name : operator==()
        // Desc : Comparison operator
        //---------------------------------------------------------------------
        bool operator!=( T t )
        {
            return (static_cast<T>(__VOIDPTR_TO_GCHANDLE(_handle).Target) != t);
        }

    private:
        //---------------------------------------------------------------------
        // Private Variables
        //---------------------------------------------------------------------
	    void* _handle;
	    
        //---------------------------------------------------------------------
        // Private Operators
        //---------------------------------------------------------------------
        T* operator& (); // Prevent taking the address.
    
    }; // End Class gcrootx
    #define gcptr( x ) cgAutoPtr<gcrootx<x>>

    //-------------------------------------------------------------------------
    // Name : cfAPI (Static Class)
    /// <summary>
    /// Static class (class with only static members) containing various
    /// support routines.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfAPI
    {
    public:
        static cgString ToNative( gcrootx<System::String^> str );
        static gcrootx<System::String^> ToManaged( const cgString & str );
        static cgUID ToNative( gcrootx<System::Guid> uid );
        static gcrootx<System::Guid> ToManaged( const cgUID & uid );
        static gcrootx<System::Drawing::Bitmap^> ToManaged( cgTexture * pTexture );
        static gcrootx<System::Drawing::Bitmap^> ToManaged( cgTexture * pTexture, const cgSize & desiredSize );
    };

    #undef __GCHANDLE_TO_VOIDPTR
    #undef __VOIDPTR_TO_GCHANDLE
    #undef __NULLPTR

} // End namespace CarbonForge