#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <System/cgString.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace System {

// Package declaration
namespace String
{
    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.System.String" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "String", sizeof(cgString), asOBJ_VALUE | asOBJ_APP_CLASS_CDA) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            // Register the factory to return a handle to a new String
	        BINDSUCCESS( engine->registerStringFactory( "String", asFUNCTION(factory), asCALL_CDECL) );

            // Register the object operator overloads
            BINDSUCCESS( engine->registerObjectBehavior( "String", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(construct), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectBehavior( "String", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(destruct), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAssign(const String &in)", asMETHODPR(cgString, operator=, (const cgString&), cgString&), asCALL_THISCALL) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAddAssign(const String &in)", asMETHODPR(cgString, operator+=, (const cgString&), cgString&), asCALL_THISCALL) );

	        // Register the global operator overloads
            {
                using namespace std;
                BINDSUCCESS( engine->registerObjectMethod( "String", "bool opEquals(const String &in) const", asFUNCTIONPR(operator==, (const basic_string<cgTChar> &, const basic_string<cgTChar> &), bool), asCALL_CDECL_OBJFIRST) );
            } // End scoped 'using namespace'
	        BINDSUCCESS( engine->registerObjectMethod( "String", "int opCmp(const String &in) const", asFUNCTION(compare), asCALL_CDECL_OBJFIRST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd(const String &in) const", asFUNCTIONPR(operator+, (const cgString &, const cgString &), cgString), asCALL_CDECL_OBJFIRST) );
            
            // Register the index operator, both as a mutator and as an inspector
            // Note that we don't register the operator[] directly as it doesn't do bounds checking
            BINDSUCCESS( engine->registerObjectMethod( "String", "uint8 &opIndex(uint)", asFUNCTION(charAt), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "const uint8 &opIndex(uint) const", asFUNCTION(charAt), asCALL_CDECL_OBJLAST) );
    	    
	        // Register the object methods
            {
                using namespace std;
                BINDSUCCESS( engine->registerObjectMethod( "String", "String & append( const String &in )", asMETHODPR(cgString,append,(const std::basic_string<cgTChar>&),std::basic_string<cgTChar>&), asCALL_THISCALL) );
            } // End scoped 'using namespace'
            BINDSUCCESS( engine->registerObjectMethod( "String", "bool empty( ) const", asMETHODPR(cgString,empty,() const,bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "String", "String & trim( )", asMETHODPR(cgString,trim,(),cgString&), asCALL_THISCALL) );
            if( sizeof(size_t) == 4 )
	        {
		        BINDSUCCESS( engine->registerObjectMethod( "String", "uint size() const", asMETHOD(cgString,size), asCALL_THISCALL) );
		        BINDSUCCESS( engine->registerObjectMethod( "String", "void resize(uint)", asMETHODPR(cgString,resize,(size_t),void), asCALL_THISCALL) );
    	    
            } // End if 32 bit
	        else
	        {
		        BINDSUCCESS( engine->registerObjectMethod( "String", "uint64 size() const", asMETHOD(cgString,size), asCALL_THISCALL) );
		        BINDSUCCESS( engine->registerObjectMethod( "String", "void resize(uint64)", asMETHODPR(cgString,resize,(size_t),void), asCALL_THISCALL) );
    	    
            } // End if 64 bit
    	    
            // Automatic conversion from values
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAssign(double)", asFUNCTION(assignDoubleToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAddAssign(double)", asFUNCTION(addAssignDoubleToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd(double) const", asFUNCTION(addStringDouble), asCALL_CDECL_OBJFIRST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd_r(double) const", asFUNCTION(addDoubleString), asCALL_CDECL_OBJLAST) );

            BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAssign(float)", asFUNCTION(assignFloatToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAddAssign(float)", asFUNCTION(addAssignFloatToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd(float) const", asFUNCTION(addStringFloat), asCALL_CDECL_OBJFIRST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd_r(float) const", asFUNCTION(addFloatString), asCALL_CDECL_OBJLAST) );

	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAssign(int)", asFUNCTION(assignIntToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAddAssign(int)", asFUNCTION(addAssignIntToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd(int) const", asFUNCTION(addStringInt), asCALL_CDECL_OBJFIRST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd_r(int) const", asFUNCTION(addIntString), asCALL_CDECL_OBJLAST) );

	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAssign(uint)", asFUNCTION(assignUIntToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String &opAddAssign(uint)", asFUNCTION(addAssignUIntToString), asCALL_CDECL_OBJLAST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd(uint) const", asFUNCTION(addStringUInt), asCALL_CDECL_OBJFIRST) );
	        BINDSUCCESS( engine->registerObjectMethod( "String", "String opAdd_r(uint) const", asFUNCTION(addUIntString), asCALL_CDECL_OBJLAST) );
        }

        //---------------------------------------------------------------------
        //  Name : construct ()
        /// <summary>
        /// This is a wrapper for the default cgString constructor, since it is
        /// not possible to take the address of the constructor directly.
        /// </summary>
        //---------------------------------------------------------------------
        static void construct( cgString *thisPointer )
        {
            // Use placement new to allocate which will in turn call the constructor
            new(thisPointer) cgString();
        }

        //---------------------------------------------------------------------
        //  Name : destruct ()
        /// <summary>
        /// A wrapper for the cgString destructor. This will be triggered
        /// whenever a string goes out of scope inside the script.
        /// </summary>
        //---------------------------------------------------------------------
        static void destruct( cgString *thisPointer )
        {
            thisPointer->~cgString();
        }

        //---------------------------------------------------------------------
        //  Name : factory ()
        /// <summary>
        /// This is the string factory that creates new strings for the script
        /// and populates them with the specified string data.
        /// </summary>
        //---------------------------------------------------------------------
        static cgString factory( cgUInt length, const cgChar *s )
        {
            STRING_CONVERT;

            // Return a script handle to a new string
            return cgString(stringConvertA2CT(s), length);
        }

        //---------------------------------------------------------------------
        //  Name : charAt ()
        /// <summary>
        /// Retrieve the character at the specified position (with bounds check).
        /// </summary>
        //---------------------------------------------------------------------
        static cgChar * charAt( cgUInt32 i, cgString &str )
        {
	        if ( i >= str.size() )
	        {
		        // Set a script exception
		        asIScriptContext * ctx = asGetActiveContext();
		        ctx->SetException("Out of range");

		        // Return a null pointer
		        return 0;
        	
            } // End if OOB

            // Return character.
            // ToDo: Should probably handle this differently instead of a direct cast.
	        return (cgChar*)&str[i];
        }

        //---------------------------------------------------------------------
        //  Name : compare ()
        /// <summary>
        /// compare the two specified strings.
        /// </summary>
        //---------------------------------------------------------------------
        static cgInt compare( const cgString &a, const cgString &b )
        {
	        int cmp = 0;
	        if ( a < b ) cmp = -1;
	        else if( a > b ) cmp = 1;
	        return cmp;
        }

        //---------------------------------------------------------------------
        //  Name : assign*ToString ()
        /// <summary>
        /// Given a value and a destination string, build a string from the
        /// numeric value.
        /// Note : Used to perform 'operator= (string,value)' via script.
        /// </summary>
        //---------------------------------------------------------------------
        static cgString & assignUIntToString( cgUInt32 i, cgString & dest )
        {
            cgStringParser stream;
            stream << i;
            dest = stream.str(); 
            return dest;
        }

        static cgString & assignIntToString( int i, cgString & dest )
        {
            cgStringParser stream;
            stream << i;
            dest = stream.str(); 
            return dest;
        }

        static cgString & assignFloatToString( cgFloat f, cgString & dest )
        {
            cgStringParser stream;
            stream << f;
            dest = stream.str(); 
            return dest;
        }

        static cgString & assignDoubleToString( cgDouble f, cgString & dest )
        {
            cgStringParser stream;
            stream << f;
            dest = stream.str(); 
            return dest;
        }

        //---------------------------------------------------------------------
        //  Name : addAssign*ToString ()
        /// <summary>
        /// Given a value and a destination string, build a string from the
        /// numeric value and concatenate the strings into the destination.
        /// Note : Used to perform 'operator+= (string,value)' via script.
        /// </summary>
        //---------------------------------------------------------------------
        static cgString & addAssignUIntToString( cgUInt32 i, cgString & dest )
        {
            cgStringParser stream;
            stream << i;
            dest += stream.str(); 
            return dest;
        }

        static cgString & addAssignIntToString( cgInt i, cgString & dest )
        {
            cgStringParser stream;
            stream << i;
            dest += stream.str(); 
            return dest;
        }

        static cgString & addAssignFloatToString( cgFloat f, cgString & dest )
        {
            cgStringParser stream;
            stream << f;
            dest += stream.str(); 
            return dest;
        }

        static cgString & addAssignDoubleToString( cgDouble f, cgString & dest )
        {
            cgStringParser stream;
            stream << f;
            dest += stream.str(); 
            return dest;
        }

        //---------------------------------------------------------------------
        //  Name : addString* ()
        /// <summary>
        /// Given two variables on either side of the addition (+) operator,
        /// this function will build a resulting string containing the string
        /// from the left side, and a value converted to a string from the
        /// right. Note : Used to perform 'operator+ (string,value)' via script
        /// </summary>
        //---------------------------------------------------------------------
        static cgString addStringUInt( const cgString & str, cgUInt32 i )
        {
            cgStringParser stream;
            stream << i;
            return str + cgString(stream.str());
        }

        static cgString addStringInt( const cgString & str, int i )
        {
            cgStringParser stream;
            stream << i;
            return str + cgString(stream.str());
        }

        static cgString addStringFloat( const cgString & str, cgFloat f )
        {
            cgStringParser stream;
            stream << f;
            return str + cgString(stream.str());
        }

        static cgString addStringDouble( const cgString & str, cgDouble f )
        {
            cgStringParser stream;
            stream << f;
            return str + cgString(stream.str());
        }

        //---------------------------------------------------------------------
        //  Name : add*String ()
        /// <summary>
        /// Given two variables on either side of the addition (+) operator,
        /// this function will build a resulting string containing the value
        /// converted to a string from the left side, and the string from the
        /// right. Note : Used to perform 'operator+ (value,string)' via script
        /// </summary>
        //---------------------------------------------------------------------
        static cgString addIntString( int i, const cgString &str )
        {
            cgStringParser stream;
            stream << i;
            return cgString(stream.str()) + str;
        }

        static cgString addUIntString( cgUInt32 i, const cgString & str )
        {
            cgStringParser stream;
            stream << i;
            return cgString(stream.str()) + str;
        }

        static cgString addFloatString( cgFloat f, const cgString & str )
        {
            cgStringParser stream;
            stream << f;
            return cgString(stream.str()) + str;
        }

        static cgString addDoubleString( cgDouble f, const cgString & str )
        {
            cgStringParser stream;
            stream << f;
            return cgString(stream.str()) + str;
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::System::String