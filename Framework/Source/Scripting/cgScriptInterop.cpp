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
// Name : cgScriptInterop.cpp                                                //
//                                                                           //
// Desc : Simple namespace containing various different utility functions    //
//        and constants for providing interoperability with the scripting    //
//        system.                                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgScriptInterop Module Includes
//-----------------------------------------------------------------------------
#include <Scripting/cgScriptInterop.h>
#include <Scripting/cgBindingUtils.h>
#include <Resources/cgScript.h>
#include "../Lib/AngelScript/source/as_tokendef.h"

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgScriptInterop::Types;
using namespace cgScriptInterop::Utils;
using namespace cgScriptInterop::Exceptions;

///////////////////////////////////////////////////////////////////////////////
// ScriptArray Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : ScriptArray () (Constructor)
/// <summary>
/// ScriptArray Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray::ScriptArray( cgUInt32 length, asIObjectType * objectType )
{
    // Initialize values to sensible defaults
    mBuffer     = CG_NULL;
	mRefCount   = 1;
	mGCFlag     = false;
	mObjectType = objectType;
	objectType->AddRef();

    // Pre-cache important information about the array.
	cacheDetails();

	// Determine the element size
	if( mSubTypeId & asTYPEID_MASK_OBJECT )
		mElementSize = sizeof(asPWORD);
	else
		mElementSize = objectType->GetEngine()->GetSizeOfPrimitiveType(mSubTypeId);

	// Make sure the array size isn't too large for us to handle
	if( !checkMaxSize(length) )
		return;
	
    // Create the array buffer
	createBuffer(&mBuffer, length);

	// Notify the GC of the successful creation
	if( objectType->GetFlags() & asOBJ_GC )
		objectType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objectType);
}

//-----------------------------------------------------------------------------
//  Name : ScriptArray () (Constructor)
/// <summary>
/// ScriptArray Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray::ScriptArray( asIObjectType * objectType, void * initBuffer)
{
    // Initialize values to sensible defaults
    mBuffer     = CG_NULL;
    mRefCount   = 1;
	mGCFlag     = false;
	mObjectType = objectType;
	objectType->AddRef();

    // Pre-cache important information about the array.
	cacheDetails();

	// Determine element size
    asIScriptEngine * engine = objectType->GetEngine();
	if( mSubTypeId & asTYPEID_MASK_OBJECT )
		mElementSize = sizeof(asPWORD);
	else
		mElementSize = engine->GetSizeOfPrimitiveType(mSubTypeId);

	// Determine the initial size from the buffer
	cgUInt32 length = *(asUINT*)initBuffer;

	// Make sure the array size isn't too large for us to handle
	if( !checkMaxSize(length) )
		return;
	
	// Copy the values of the array elements from the buffer
	if( (objectType->GetSubTypeId() & asTYPEID_MASK_OBJECT) == 0 )
	{
		createBuffer(&mBuffer, length);

		// Copy the values of the primitive type into the internal buffer
		memcpy(at(0), (((asUINT*)initBuffer)+1), length * mElementSize);
	}
	else if( objectType->GetSubTypeId() & asTYPEID_OBJHANDLE )
	{
		createBuffer(&mBuffer, length);

		// Copy the handles into the internal buffer
		memcpy(at(0), (((asUINT*)initBuffer)+1), length * mElementSize);

		// With object handles it is safe to clear the memory in the received buffer
		// instead of increasing the ref count. It will save time both by avoiding the
		// call the increase ref, and also relieve the engine from having to release
		// its references too
		memset((((asUINT*)initBuffer)+1), 0, length * mElementSize);
	}
	else if( objectType->GetSubType()->GetFlags() & asOBJ_REF )
	{
		// Only allocate the buffer, but not the objects
		mSubTypeId |= asTYPEID_OBJHANDLE;
		createBuffer(&mBuffer, length);
		mSubTypeId &= ~asTYPEID_OBJHANDLE;

		// Copy the handles into the internal buffer
		memcpy(mBuffer->data, (((asUINT*)initBuffer)+1), length * mElementSize);

		// For ref types we can do the same as for handles, as they are
		// implicitly stored as handles.
		memset((((asUINT*)initBuffer)+1), 0, length * mElementSize);
	}
	else
	{
		// TODO: Optimize by calling the copy constructor of the object instead of
		//       constructing with the default constructor and then assigning the value
		// TODO: With C++11 ideally we should be calling the move constructor, instead
		//       of the copy constructor as the engine will just discard the objects in the
		//       buffer afterwards.
		createBuffer(&mBuffer, length);

		// For value types we need to call the opAssign for each individual object
		for ( cgUInt32 n = 0; n < length; n++ )
		{
			void * object = at(n);
			cgByte * srcObject = (cgByte*)initBuffer;
			srcObject += 4 + n * objectType->GetSubType()->GetSize();
			engine->AssignScriptObject(object, srcObject, objectType->GetSubType());
		
        } // Next element
	}

	// Notify the GC of the successful creation
	if( objectType->GetFlags() & asOBJ_GC )
		objectType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objectType);
}

//-----------------------------------------------------------------------------
//  Name : ScriptArray () (Constructor)
/// <summary>
/// ScriptArray Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray::ScriptArray( const ScriptArray & other )
{
    // Initialize values to sensible defaults
	mRefCount   = 1;
	mGCFlag     = false;
	mObjectType = other.mObjectType;
	mObjectType->AddRef();

	// Pre-cache important information about the array.
	cacheDetails();

    // Duplicate other important details.
	mElementSize = other.mElementSize;

    // Notify the GC of the successful creation
	if( mObjectType->GetFlags() & asOBJ_GC )
		mObjectType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, mObjectType);

    // Create the initial buffer so that it can be resized.
	createBuffer(&mBuffer, 0);

	// Copy the content
	*this = other;
}

//-----------------------------------------------------------------------------
//  Name : ScriptArray () (Constructor)
/// <summary>
/// ScriptArray Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray::ScriptArray( cgUInt32 length, void * defaultValue, asIObjectType * objectType)
{
    // Initialize values to sensible defaults
    mBuffer     = CG_NULL;
	mRefCount   = 1;
	mGCFlag     = false;
	mObjectType = objectType;
	objectType->AddRef();

	// Pre-cache important information about the array.
	cacheDetails();

	// Determine element size
	if( mSubTypeId & asTYPEID_MASK_OBJECT )
		mElementSize = sizeof(asPWORD);
	else
		mElementSize = objectType->GetEngine()->GetSizeOfPrimitiveType(mSubTypeId);

	// Make sure the array size isn't too large for us to handle
	if( !checkMaxSize(length) )
		return;

    // Create the array buffer
	createBuffer( &mBuffer, length );

	// Notify the GC of the successful creation
	if( objectType->GetFlags() & asOBJ_GC )
		objectType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, objectType);

	// Initialize the elements with the default value
	for( cgUInt32 n = 0; n < getSize(); ++n )
		setValue( n, defaultValue );
}

//-----------------------------------------------------------------------------
//  Name : ~ScriptArray () (Destructor)
/// <summary>
/// ScriptArray Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray::~ScriptArray()
{
    if ( mBuffer )
        deleteBuffer( mBuffer );
    mBuffer = CG_NULL;
    
    if ( mObjectType )
        mObjectType->Release();
    if ( mSubType )
        mSubType->Release();
}

//-----------------------------------------------------------------------------
//  Name : factory () (Static)
/// <summary>
/// Construct a new instance of the ScriptArray class.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray * ScriptArray::factory( asIObjectType * objectType )
{
    return new ScriptArray(0, objectType);
}

//-----------------------------------------------------------------------------
//  Name : factory () (Static)
/// <summary>
/// Construct a new instance of the ScriptArray class with the specified
/// length.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray * ScriptArray::factory( asIObjectType * objectType, cgUInt32 length )
{
    return new ScriptArray( length, objectType );
}

//-----------------------------------------------------------------------------
//  Name : factory () (Static)
/// <summary>
/// Construct a new instance of the ScriptArray class with the specified
/// length and default element values.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray * ScriptArray::factory( asIObjectType * objectType, cgUInt32 length, void * defaultValue )
{
	ScriptArray *a = new ScriptArray( length, defaultValue, objectType );

	// It's possible the constructor raised a script exception, in which case we
	// need to free the memory and return null instead, else we get a memory leak.
	asIScriptContext *ctx = asGetActiveContext();
	if( ctx && ctx->GetState() == asEXECUTION_EXCEPTION )
	{
		a->release();
		return 0;
	}

	return a;
}

//-----------------------------------------------------------------------------
//  Name : listFactory () (Static)
/// <summary>
/// Construct a new instance of the ScriptArray class with the specified
/// length used exclusively for initialization lists.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray * ScriptArray::listFactory( asIObjectType * objectType, void * initList )
{
	ScriptArray * a = new ScriptArray( objectType, initList );

	// It's possible the constructor raised a script exception, in which case we 
	// need to free the memory and return null instead, else we get a memory leak.
	asIScriptContext * ctx = asGetActiveContext();
	if ( ctx && ctx->GetState() == asEXECUTION_EXCEPTION )
	{
		a->release();
		return 0;
	
    } // End if exception
	return a;
}

//-----------------------------------------------------------------------------
//  Name : templateCallback () (Static)
/// <summary>
/// Validate whether the specified subtype is valid for this template.
/// </summary>
//-----------------------------------------------------------------------------
bool ScriptArray::templateCallback(asIObjectType *ot, bool &dontGarbageCollect)
{
    // This optional callback is called when the template type is first used by the compiler.
    // It allows the application to validate if the template can be instanciated for the requested 
    // subtype at compile time, instead of at runtime. The output argument dontGarbageCollect
    // allow the callback to tell the engine if the template instance type shouldn't be garbage collected, 
    // i.e. no asOBJ_GC flag. 

	// Make sure the subtype can be instanciated with a default factory/constructor, 
	// otherwise we won't be able to instanciate the elements. 
	int typeId = ot->GetSubTypeId();
	if( typeId == asTYPEID_VOID )
		return false;
	if( (typeId & asTYPEID_MASK_OBJECT) && !(typeId & asTYPEID_OBJHANDLE) )
	{
		asIObjectType *subtype = ot->GetEngine()->GetObjectTypeById(typeId);
		asDWORD flags = subtype->GetFlags();
		if( (flags & asOBJ_VALUE) && !(flags & asOBJ_POD) )
		{
			// Verify that there is a default constructor
			bool found = false;
			for( asUINT n = 0; n < subtype->GetBehaviourCount(); n++ )
			{
				asEBehaviours beh;
				asIScriptFunction *func = subtype->GetBehaviourByIndex(n, &beh);
				if( beh != asBEHAVE_CONSTRUCT ) continue;

				if( func->GetParamCount() == 0 )
				{
					// Found the default constructor
					found = true;
					break;
				}
			}

			if( !found )
			{
				// There is no default constructor
				return false;
			}
		}
		else if( (flags & asOBJ_REF) )
		{
			// Verify that there is a default factory
			bool found = false;
			for( asUINT n = 0; n < subtype->GetFactoryCount(); n++ )
			{
				asIScriptFunction *func = subtype->GetFactoryByIndex(n);
				if( func->GetParamCount() == 0 )
				{
					// Found the default factory
					found = true;
					break;
				}
			}	

			if( !found )
			{
				// No default factory
				return false;
			}
		}

		// If the object type is not garbage collected then the array also doesn't need to be
		if( !(flags & asOBJ_GC) )
			dontGarbageCollect = true;
	}
	else if( !(typeId & asTYPEID_OBJHANDLE) )
	{
		// Arrays with primitives cannot form circular references, 
		// thus there is no need to garbage collect them
		dontGarbageCollect = true;
	}

	// The type is ok
	return true;
}

//-----------------------------------------------------------------------------
//  Name : bind () (Static)
/// <summary>
/// Register the script array template type.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::bind( asIScriptEngine * pEngine )
{
    // Register the array type as a template
    BINDSUCCESS( pEngine->RegisterObjectType("array<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE) );

    // Register a callback for validating the subtype before it is used
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(templateCallback), asCALL_CDECL) );

    // Templates receive the object type as the first parameter. To the script writer this is hidden
    BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in)", asFUNCTIONPR(factory, (asIObjectType*), ScriptArray*), asCALL_CDECL) );
    BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint)", asFUNCTIONPR(factory, (asIObjectType*, cgUInt32), ScriptArray*), asCALL_CDECL) );
    BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_FACTORY, "array<T>@ f(int&in, uint, const T&in)", asFUNCTIONPR(factory, (asIObjectType*, cgUInt32, void*), ScriptArray*), asCALL_CDECL) );

    // Register the factory that will be used for initialization lists
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_LIST_FACTORY, "array<T>@ f(int&in, int&in list) {repeat T}", asFUNCTIONPR(listFactory, (asIObjectType*, void*), ScriptArray*), asCALL_CDECL) );

    // The memory management methods
    BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_ADDREF, "void f()", asMETHOD(ScriptArray,addRef), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_RELEASE, "void f()", asMETHOD(ScriptArray,release), asCALL_THISCALL) );

    // Register GC behaviours in case the array needs to be garbage collected
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(ScriptArray, getRefCount), asCALL_THISCALL) );
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_SETGCFLAG, "void f()", asMETHOD(ScriptArray, setGCFlag), asCALL_THISCALL) );
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(ScriptArray, getGCFlag), asCALL_THISCALL) );
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(ScriptArray, enumReferences), asCALL_THISCALL) );
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(ScriptArray, releaseAllHandles), asCALL_THISCALL) );

    // The index operator returns the template subtype
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "T &opIndex(uint)", asMETHODPR(ScriptArray, at, (cgUInt32), void*), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "const T &opIndex(uint) const", asMETHODPR(ScriptArray, at, (cgUInt32) const, const void*), asCALL_THISCALL) );
    
    // The assignment operator
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "array<T> &opAssign(const array<T>&in)", asMETHOD(ScriptArray, operator=), asCALL_THISCALL) );

    // Equality operator
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "bool opEquals(const array<T>&in) const", asMETHOD(ScriptArray, operator==), asCALL_THISCALL) );

    // Other methods
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void insertAt(uint, const T&in)", asMETHOD(ScriptArray, insertAt), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void removeAt(uint)", asMETHOD(ScriptArray, removeAt), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void insertLast(const T&in)", asMETHOD(ScriptArray, insertLast), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void removeLast()", asMETHOD(ScriptArray, removeLast), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "uint length() const", asMETHOD(ScriptArray, getSize), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void reserve(uint)", asMETHOD(ScriptArray, reserve), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void resize(uint)", asMETHODPR(ScriptArray, resize, (cgUInt32), void), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void sortAsc()", asMETHODPR(ScriptArray, sortAsc, (), void), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void sortAsc(uint,uint)", asMETHODPR(ScriptArray, sortAsc, (cgUInt32,cgUInt32), void), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void sortDesc()", asMETHODPR(ScriptArray, sortDesc, (), void), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void sortDesc(uint,uint)", asMETHODPR(ScriptArray, sortDesc, (cgUInt32,cgUInt32), void), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void reverse()", asMETHOD(ScriptArray, reverse), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "int find(const T&in) const", asMETHODPR(ScriptArray, find, (void*)const, cgInt32), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "int find(uint, const T&in) const", asMETHODPR(ScriptArray, find, (cgUInt32, void*)const, cgInt32), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "int findByRef(const T&in) const", asMETHODPR(ScriptArray, findByRef, (void*)const, cgInt32), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "int findByRef(uint, const T&in) const", asMETHODPR(ScriptArray, findByRef, (cgUInt32, void*)const, cgInt32), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "bool isEmpty() const", asMETHOD(ScriptArray, isEmpty), asCALL_THISCALL) );

    // Virtual Properties
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "uint get_length() const", asMETHOD(ScriptArray, getSize), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void set_length(uint)", asMETHODPR(ScriptArray, resize, (cgUInt32), void), asCALL_THISCALL) );

    // Register STL compatible method name aliases.
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "uint size() const", asMETHOD(ScriptArray, getSize), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "bool empty() const", asMETHOD(ScriptArray, isEmpty), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void push_back(const T&in)", asMETHOD(ScriptArray, insertLast), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void pop_back()", asMETHOD(ScriptArray, removeLast), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void insert(uint, const T&in)", asMETHOD(ScriptArray, insertAt), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void erase(uint)", asMETHOD(ScriptArray, removeAt), asCALL_THISCALL) );
    
}

//-----------------------------------------------------------------------------
//  Name : operator= ()
/// <summary>
/// Assignment operator for this class.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray &ScriptArray::operator=( const ScriptArray & other )
{
    // Only perform the copy if the array types are the same
    if ( &other != this && other.getArrayObjectType() == getArrayObjectType() )
    {
        // Make sure the arrays are of the same size
		resize(other.mBuffer->elementCount);

        // Copy all elements from the other array
        copyBuffer( mBuffer, other.mBuffer);
    
    } // End if matching types

    return *this;
}

//-----------------------------------------------------------------------------
//  Name : getSize ()
/// <summary>
/// Retrieve the number of elements stored in this array.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ScriptArray::getSize() const
{
    return mBuffer->elementCount;
}

//-----------------------------------------------------------------------------
//  Name : isEmpty ()
/// <summary>
/// Determine if the array is currently empty (0 elements).
/// </summary>
//-----------------------------------------------------------------------------
bool ScriptArray::isEmpty() const
{
    return mBuffer->elementCount == 0;
}

//-----------------------------------------------------------------------------
//  Name : reserve ()
/// <summary>
/// Reserve space in the internal array buffer for the specified number of
/// elements, but do not create/add elements.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::reserve( cgUInt32 maxElements )
{
    // Cannot shrink.
	if ( maxElements <= mBuffer->maxElements )
		return;

    // Make sure we can allocate.
	if( !checkMaxSize(maxElements) )
		return;

	// Allocate memory for the buffer
    ArrayBuffer * newBuffer = (ArrayBuffer*)new (std::nothrow) cgByte[sizeof(ArrayBuffer)-1 + mElementSize*maxElements];
	if ( newBuffer )
	{
		newBuffer->elementCount = mBuffer->elementCount;
		newBuffer->maxElements = maxElements;
	
    } // End if success
	else
	{
		// Out of memory
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Out of memory");
		return;
	
    } // End if out of memory

    // Copy data from old buffer into new buffer.
	memcpy(newBuffer->data, mBuffer->data, mBuffer->elementCount*mElementSize);

	// Release the old buffer
	delete[] (cgByte*)mBuffer;

    // Use the new buffer.
	mBuffer = newBuffer;
}

//-----------------------------------------------------------------------------
//  Name : resize ()
/// <summary>
/// Resize the array such that it is capable of storing the specified number
/// of elements.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::resize( cgUInt32 elementCount )
{
    // Make sure we can store this many elements.
    if ( !checkMaxSize(elementCount) )
		return;

    // Pass through to primary method.
	resize((cgInt)elementCount - (cgInt)mBuffer->elementCount, (cgUInt)-1);
}

//-----------------------------------------------------------------------------
//  Name : resize () (Protected)
/// <summary>
/// Resize the array and insert new elements / remove existing elements at the
/// specified location.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::resize( cgInt delta, cgUInt operateAt )
{
    if ( delta < 0 )
	{
		if( -delta > (cgInt)mBuffer->elementCount )
			delta = -(cgInt)mBuffer->elementCount;
		if( operateAt > mBuffer->elementCount + delta )
			operateAt = mBuffer->elementCount + delta;
	}
	else if ( delta > 0 )
	{
		// Make sure the array size isn't too large for us to handle
		if( delta > 0 && !checkMaxSize(mBuffer->elementCount + delta) )
			return;

		if( operateAt > mBuffer->elementCount )
			operateAt = mBuffer->elementCount;
	}

    // Is this a no-op?
	if ( delta == 0 )
        return;

	if ( mBuffer->maxElements < mBuffer->elementCount + delta )
	{
		// Allocate memory for the buffer
		ArrayBuffer * newBuffer = (ArrayBuffer*)new (std::nothrow) cgByte[sizeof(ArrayBuffer)-1 + mElementSize*(mBuffer->elementCount + delta)];
		if ( newBuffer )
		{
			newBuffer->elementCount = mBuffer->elementCount + delta;
			newBuffer->maxElements = newBuffer->elementCount;
		}
		else
		{
			// Out of memory
			asIScriptContext *ctx = asGetActiveContext();
			if ( ctx )
				ctx->SetException("Out of memory");
			return;
		}

		// TODO: memcpy assumes the objects in the array doesn't hold pointers to themselves
		//       This should really be using the objects copy constructor to copy each object
		//       to the new location. It would most likely be a hit on the performance though.
		memcpy(newBuffer->data, mBuffer->data, operateAt*mElementSize);
		if ( operateAt < mBuffer->elementCount )
			memcpy(newBuffer->data + (operateAt+delta)*mElementSize, mBuffer->data + operateAt*mElementSize, (mBuffer->elementCount-operateAt)*mElementSize);

		if ( mSubTypeId & asTYPEID_MASK_OBJECT )
			construct( newBuffer, operateAt, operateAt+delta);

		// Release the old buffer
		delete[] (cgByte*)mBuffer;

        // Use the new buffer
		mBuffer = newBuffer;
	}
	else if( delta < 0 )
	{
		destruct(mBuffer, operateAt, operateAt-delta);
		// TODO: memmove assumes the objects in the array doesn't hold pointers to themselves
		//       This should really be using the objects copy constructor to copy each object
		//       to the new location. It would most likely be a hit on the performance though.
		memmove(mBuffer->data + operateAt*mElementSize, mBuffer->data + (operateAt-delta)*mElementSize, (mBuffer->elementCount - (operateAt-delta))*mElementSize);
		mBuffer->elementCount += delta;
	}
	else
	{
		// TODO: memmove assumes the objects in the array doesn't hold pointers to themselves
		//       This should really be using the objects copy constructor to copy each object
		//       to the new location. It would most likely be a hit on the performance though.
		memmove(mBuffer->data + (operateAt+delta)*mElementSize, mBuffer->data + operateAt*mElementSize, (mBuffer->elementCount - operateAt)*mElementSize);
		construct(mBuffer, operateAt, operateAt+delta);
		mBuffer->elementCount += delta;
	}
}

//-----------------------------------------------------------------------------
//  Name : checkMaxSize () (Protected)
/// <summary>
/// Make sure that the size of the buffer that is allocated for the array
/// doesn't overflow and become smaller than requested.
/// </summary>
//-----------------------------------------------------------------------------
bool ScriptArray::checkMaxSize( cgUInt32 elementCount )
{
	cgUInt32 maxSize = 0xFFFFFFFFul - sizeof(ArrayBuffer) + 1;
	if( mSubTypeId & asTYPEID_MASK_OBJECT )
		maxSize /= sizeof(void*);
	else if( mElementSize > 0 )
		maxSize /= mElementSize;

	if( elementCount > maxSize )
	{
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Array size too large");
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
//  Name : getArrayObjectType ()
/// <summary>
/// Retrieve information about the array object type.
/// </summary>
//-----------------------------------------------------------------------------
asIObjectType * ScriptArray::getArrayObjectType() const
{
    return mObjectType;
}

//-----------------------------------------------------------------------------
//  Name : getArrayTypeId ()
/// <summary>
/// Retrieve the type identifier for this array.
/// </summary>
//-----------------------------------------------------------------------------
cgInt ScriptArray::getArrayTypeId() const
{
    return mObjectType->GetTypeId();
}

//-----------------------------------------------------------------------------
//  Name : getElementTypeId ()
/// <summary>
/// Retrieve the type identifier for the elements contained in this array.
/// </summary>
//-----------------------------------------------------------------------------
cgInt ScriptArray::getElementTypeId() const
{
    return mSubTypeId;
}

//-----------------------------------------------------------------------------
//  Name : insertAt ()
/// <summary>
/// Insert the provided value at the specified location in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::insertAt( cgUInt32 index, void * value )
{
	if ( index > mBuffer->elementCount )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Index out of bounds");
		return;
	}

	// Make room for the new element
	resize(1, index);

	// Set the value of the new element
	setValue(index, value);
}

//-----------------------------------------------------------------------------
//  Name : insertLast ()
/// <summary>
/// Insert the provided value at the end of the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::insertLast( void * value )
{
	insertAt(mBuffer->elementCount, value);
}

//-----------------------------------------------------------------------------
//  Name : removeAt ()
/// <summary>
/// Remove the element at the specified location in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::removeAt( cgUInt32 index )
{
	if ( index >= mBuffer->elementCount )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Index out of bounds");
		return;
	}

	// Remove the element
	resize(-1, index);
}

//-----------------------------------------------------------------------------
//  Name : removeLast ()
/// <summary>
/// Remove the final element in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::removeLast()
{
	removeAt(mBuffer->elementCount-1);
}

//-----------------------------------------------------------------------------
//  Name : at ()
/// <summary>
/// Retrieve a pointer to the array element, or CG_NULL if out of bounds.
/// </summary>
//-----------------------------------------------------------------------------
const void * ScriptArray::at( cgUInt32 index ) const
{
	if ( mBuffer == 0 || index >= mBuffer->elementCount )
	{
		// If this is called from a script we raise a script exception
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Index out of bounds");
		return 0;
	}

	if( (mSubTypeId & asTYPEID_MASK_OBJECT) && !(mSubTypeId & asTYPEID_OBJHANDLE) )
		return (void*)((size_t*)mBuffer->data)[index];
	else
		return mBuffer->data + mElementSize*index;
}

//-----------------------------------------------------------------------------
//  Name : at ()
/// <summary>
/// Retrieve a pointer to the array element, or CG_NULL if out of bounds.
/// </summary>
//-----------------------------------------------------------------------------
void * ScriptArray::at( cgUInt32 index )
{
    return const_cast<void*>(const_cast<const ScriptArray *>(this)->at(index));
}

//-----------------------------------------------------------------------------
//  Name : setValue ()
/// <summary>
/// Set the value of an existing element of this array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::setValue( cgUInt32 index, void * value )
{
	// At() will take care of the out-of-bounds checking, though  
	// if called from the application then nothing will be done
	void * ptr = at(index);
	if ( !ptr )
        return;

	if ( (mSubTypeId & ~asTYPEID_MASK_SEQNBR) && !(mSubTypeId & asTYPEID_OBJHANDLE) )
    {
		mObjectType->GetEngine()->AssignScriptObject(ptr, value, mSubType);
    }
	else if ( mSubTypeId & asTYPEID_OBJHANDLE )
	{
		void *tmp = *(void**)ptr;
		*(void**)ptr = *(void**)value;
		mObjectType->GetEngine()->AddRefScriptObject(*(void**)value, mObjectType->GetSubType());
		if ( tmp )
			mObjectType->GetEngine()->ReleaseScriptObject(tmp, mObjectType->GetSubType());
	}
	else if ( mSubTypeId == asTYPEID_BOOL ||
			  mSubTypeId == asTYPEID_INT8 ||
			  mSubTypeId == asTYPEID_UINT8 )
		*(char*)ptr = *(char*)value;
	else if ( mSubTypeId == asTYPEID_INT16 ||
			  mSubTypeId == asTYPEID_UINT16 )
		*(short*)ptr = *(short*)value;
	else if ( mSubTypeId == asTYPEID_INT32 ||
			  mSubTypeId == asTYPEID_UINT32 ||
			  mSubTypeId == asTYPEID_FLOAT ||
			  mSubTypeId > asTYPEID_DOUBLE ) // enums have a type id larger than doubles
		*(int*)ptr = *(int*)value;
	else if ( mSubTypeId == asTYPEID_INT64 ||
			  mSubTypeId == asTYPEID_UINT64 ||
			  mSubTypeId == asTYPEID_DOUBLE )
		*(double*)ptr = *(double*)value;
}

//-----------------------------------------------------------------------------
//  Name : setSizeUninitialized ()
/// <summary>
/// Create the array's internal buffer, but do not construct its values.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::setSizeUninitialized( cgUInt32 elementCount )
{
    // Make sure we can store this many elements.
    if ( !checkMaxSize(elementCount) )
		return;

    // Release prior buffer.
    if ( mBuffer )
        deleteBuffer( mBuffer );
    mBuffer = CG_NULL;

    // Create new buffer
    createBuffer( &mBuffer, elementCount, false );
}

//-----------------------------------------------------------------------------
//  Name : createBuffer () (Protected)
/// <summary>
/// Allocate the array's internal buffer.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::createBuffer( ArrayBuffer **buffer, cgUInt32 elementCount, bool initialized /* = true */ )
{
    if ( mSubTypeId & asTYPEID_MASK_OBJECT )
        *buffer = (ArrayBuffer*)new (std::nothrow) cgByte[sizeof(ArrayBuffer)-1+sizeof(void*)*elementCount];
    else
        *buffer = (ArrayBuffer*)new (std::nothrow) cgByte[sizeof(ArrayBuffer)-1+mElementSize*elementCount];
    
    if( *buffer )
	{
		(*buffer)->elementCount = elementCount;
		(*buffer)->maxElements = elementCount;
        construct(*buffer, 0, elementCount, initialized );
	}
	else
	{
		// Oops, out of memory
		asIScriptContext *ctx = asGetActiveContext();
		if( ctx )
			ctx->SetException("Out of memory");
	}
}

//-----------------------------------------------------------------------------
//  Name : deleteBuffer () (Protected)
/// <summary>
/// Destroy the array's internal buffer.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::deleteBuffer( ArrayBuffer *buffer )
{
    // Destruct elements.
    destruct( buffer, 0, buffer->elementCount );

    // Free the buffer
    delete[] (cgByte*)buffer;
}

//-----------------------------------------------------------------------------
//  Name : construct () (Protected)
/// <summary>
/// Call the constructor for specified elements in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::construct( ArrayBuffer * buffer, cgUInt32 start, cgUInt32 end, bool initialized /* = true */ )
{
    if( mSubTypeId & asTYPEID_OBJHANDLE )
	{
		// Set all object handles to null
		void *d = (void*)(buffer->data + start * sizeof(void*));
		memset(d, 0, (end-start)*sizeof(void*));
	}
	else if( mSubTypeId & asTYPEID_MASK_OBJECT )
	{
		void **max = (void**)(buffer->data + end * sizeof(void*));
		void **d = (void**)(buffer->data + start * sizeof(void*));

		asIScriptEngine *engine = mObjectType->GetEngine();
        if ( initialized )
        {
		    for( ; d < max; d++ )
			    *d = (void*)engine->CreateScriptObject(mSubType);
        }
        else
        {
            for( ; d < max; d++ )
			    *d = (void*)engine->CreateUninitializedScriptObject(mSubType);
        }
	}
}

//-----------------------------------------------------------------------------
//  Name : destruct () (Protected)
/// <summary>
/// Call the destructor for specified elements in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::destruct( ArrayBuffer * buffer, cgUInt32 start, cgUInt32 end )
{
    if( mSubTypeId & asTYPEID_MASK_OBJECT )
	{
		asIScriptEngine *engine = mObjectType->GetEngine();
		void **max = (void**)(buffer->data + end * sizeof(void*));
		void **d   = (void**)(buffer->data + start * sizeof(void*));

		for( ; d < max; d++ )
		{
			if( *d )
				engine->ReleaseScriptObject(*d, mSubType);
		}
	}
}

//-----------------------------------------------------------------------------
//  Name : less () (Protected)
/// <summary>
/// Perform a 'less than' comparison on the provided array values.
/// </summary>
//-----------------------------------------------------------------------------
bool ScriptArray::less(const void *a, const void *b, bool ascending, asIScriptContext *context, ArrayCache *cache)
{
	if ( !ascending )
	{
		// Swap items
		const void *temp = a;
		a = b;
		b = temp;
	}

	if( !(mSubTypeId & ~asTYPEID_MASK_SEQNBR) )
	{
		// Simple compare of values
		switch( mSubTypeId )
		{
			#define ASARRCOMPARE(T) *((T*)a) < *((T*)b)
			case asTYPEID_BOOL: return ASARRCOMPARE(bool);
			case asTYPEID_INT8: return ASARRCOMPARE(signed char);
			case asTYPEID_UINT8: return ASARRCOMPARE(unsigned char);
			case asTYPEID_INT16: return ASARRCOMPARE(signed short);
			case asTYPEID_UINT16: return ASARRCOMPARE(unsigned short);
			case asTYPEID_INT32: return ASARRCOMPARE(signed int);
			case asTYPEID_UINT32: return ASARRCOMPARE(unsigned int);
			case asTYPEID_FLOAT: return ASARRCOMPARE(float);
			case asTYPEID_DOUBLE: return ASARRCOMPARE(double);
			default: return ASARRCOMPARE(signed int); // All enums fall in this case
			#undef ASARRCOMPARE
		}
	}
	else
	{
		cgInt r = 0;
		if( mSubTypeId & asTYPEID_OBJHANDLE )
		{
			// Allow sort to work even if the array contains null handles
			if( *(void**)a == 0 ) return true;
			if( *(void**)b == 0 ) return false;
		}

		// Execute object opCmp
		if ( cache && cache->cmpFunc )
		{
			// TODO: Add proper error handling
			r = context->Prepare(cache->cmpFunc); cgAssert(r >= 0);

			if( mSubTypeId & asTYPEID_OBJHANDLE )
			{
				r = context->SetObject(*((void**)a)); cgAssert(r >= 0);
				r = context->SetArgObject(0, *((void**)b)); cgAssert(r >= 0);
			}
			else
			{
				r = context->SetObject((void*)a); cgAssert(r >= 0);
				r = context->SetArgObject(0, (void*)b); cgAssert(r >= 0);
			}

			r = context->Execute();

			if( r == asEXECUTION_FINISHED )
				return (int)context->GetReturnDWord() < 0;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
//  Name : reverse ()
/// <summary>
/// Reverse the elements in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::reverse()
{
	cgUInt32 size = getSize();
	if ( size >= 2 )
	{
		cgByte temp[16];
		for ( cgUInt32 i = 0; i < size / 2; i++ )
		{
			copy(temp, getArrayItemPointer(i));
			copy(getArrayItemPointer(i), getArrayItemPointer(size - i - 1));
			copy(getArrayItemPointer(size - i - 1), temp);
		
        } // Next element
	}
}

//-----------------------------------------------------------------------------
//  Name : operator== ()
/// <summary>
/// Test the two arrays for equivalency.
/// </summary>
//-----------------------------------------------------------------------------
bool ScriptArray::operator==(const ScriptArray &other) const
{
	if( mObjectType != other.mObjectType )
		return false;
	if( getSize() != other.getSize() )
		return false;

	asIScriptContext *cmpContext = 0;
	bool isNested = false;
	if ( mSubTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if ( cmpContext )
		{
			if( cmpContext->GetEngine() == mObjectType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = mObjectType->GetEngine()->CreateContext();
		}
	}

	// Check if all elements are equal
	bool isEqual = true;
	ArrayCache *cache = reinterpret_cast<ArrayCache*>(mObjectType->GetUserData(ARRAY_CACHE));
	for ( cgUInt32 n = 0; n < getSize(); ++n )
    {
		if ( !equals(at(n), other.at(n), cmpContext, cache) )
		{
			isEqual = false;
			break;
		}
    }

	if ( cmpContext )
    {
		if( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
    }

	return isEqual;
}

//-----------------------------------------------------------------------------
//  Name : equals () (Protected)
/// <summary>
/// Perform an 'equality' comparison on the provided array values.
/// </summary>
//-----------------------------------------------------------------------------
bool ScriptArray::equals( const void *a, const void *b, asIScriptContext *context, ArrayCache *cache ) const
{
	if( !(mSubTypeId & ~asTYPEID_MASK_SEQNBR) )
	{
		// Simple compare of values
		switch( mSubTypeId )
		{
			#define ASARRCOMPARE(T) *((T*)a) == *((T*)b)
			case asTYPEID_BOOL: return ASARRCOMPARE(bool);
			case asTYPEID_INT8: return ASARRCOMPARE(signed char);
			case asTYPEID_UINT8: return ASARRCOMPARE(unsigned char);
			case asTYPEID_INT16: return ASARRCOMPARE(signed short);
			case asTYPEID_UINT16: return ASARRCOMPARE(unsigned short);
			case asTYPEID_INT32: return ASARRCOMPARE(signed int);
			case asTYPEID_UINT32: return ASARRCOMPARE(unsigned int);
			case asTYPEID_FLOAT: return ASARRCOMPARE(float);
			case asTYPEID_DOUBLE: return ASARRCOMPARE(double);
			default: return ASARRCOMPARE(signed int); // All enums fall here
			#undef ASARRCOMPARE
		}
	}
	else
	{
		int r = 0;
		if ( mSubTypeId & asTYPEID_OBJHANDLE )
		{
			// Allow the find to work even if the array contains null handles
			if( *(void**)a == *(void**)b ) return true;
		}

		// Execute object opEquals if available
		if ( cache && cache->eqFunc )
		{
			// TODO: Add proper error handling
			r = context->Prepare(cache->eqFunc); cgAssert(r >= 0);

			if( mSubTypeId & asTYPEID_OBJHANDLE )
			{
				r = context->SetObject(*((void**)a)); cgAssert(r >= 0);
				r = context->SetArgObject(0, *((void**)b)); cgAssert(r >= 0);
			}
			else
			{
				r = context->SetObject((void*)a); cgAssert(r >= 0);
				r = context->SetArgObject(0, (void*)b); cgAssert(r >= 0);
			}

			r = context->Execute();

			if( r == asEXECUTION_FINISHED )
				return context->GetReturnByte() != 0;

			return false;
		}

		// Execute object opCmp if available
		if( cache && cache->cmpFunc )
		{
			// TODO: Add proper error handling
			r = context->Prepare(cache->cmpFunc); cgAssert(r >= 0);

			if ( mSubTypeId & asTYPEID_OBJHANDLE )
			{
				r = context->SetObject(*((void**)a)); cgAssert(r >= 0);
				r = context->SetArgObject(0, *((void**)b)); cgAssert(r >= 0);
			}
			else
			{
				r = context->SetObject((void*)a); cgAssert(r >= 0);
				r = context->SetArgObject(0, (void*)b); cgAssert(r >= 0);
			}

			r = context->Execute();

			if( r == asEXECUTION_FINISHED )
				return (int)context->GetReturnDWord() == 0;

			return false;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
//  Name : findByRef ()
/// <summary>
/// Find the index of a matching element in the array based on its reference
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 ScriptArray::findByRef( void * ref ) const
{
	return findByRef(0, ref);
}

//-----------------------------------------------------------------------------
//  Name : findByRef ()
/// <summary>
/// Find the index of a matching element in the array based on its reference
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 ScriptArray::findByRef( cgUInt32 startAt, void * ref ) const
{
	cgUInt32 size = getSize();
	if( mSubTypeId & asTYPEID_OBJHANDLE )
	{
		// Dereference the pointer
		ref = *(void**)ref;
		for( cgUInt32 i = startAt; i < size; i++ )
		{
			if( *(void**)at(i) == ref )
				return i;
		}
	}
	else
	{
		// Compare the reference directly
		for( cgUInt32 i = startAt; i < size; i++ )
		{
			if( at(i) == ref )
				return i;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
//  Name : find ()
/// <summary>
/// Find the index of a matching element in the array based on its value
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 ScriptArray::find( void * value ) const
{
	return find(0, value);
}

//-----------------------------------------------------------------------------
//  Name : find ()
/// <summary>
/// Find the index of a matching element in the array based on its value
/// </summary>
//-----------------------------------------------------------------------------
cgInt32 ScriptArray::find( cgUInt32 startAt, void * value ) const
{
	// Check if the subtype really supports find()
	// TODO: Can't this be done at compile time too by the template callback
	ArrayCache * cache = 0;
	if ( mSubTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		cache = reinterpret_cast<ArrayCache*>(mObjectType->GetUserData(ARRAY_CACHE));
		if ( !cache || (cache->cmpFunc == 0 && cache->eqFunc == 0) )
		{
			asIScriptContext * ctx = asGetActiveContext();
			
			// Throw an exception
			if ( ctx )
			{
				char tmp[512];

				if ( cache && cache->eqFuncReturnCode == asMULTIPLE_FUNCTIONS )
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' has multiple matching opEquals or opCmp methods", mSubType->GetName());
#else
					sprintf(tmp, "Type '%s' has multiple matching opEquals or opCmp methods", mSubType->GetName());
#endif
				else
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' does not have a matching opEquals or opCmp method", mSubType->GetName());
#else
					sprintf(tmp, "Type '%s' does not have a matching opEquals or opCmp method", mSubType->GetName());
#endif
				ctx->SetException(tmp);
			}

			return -1;
		}
	}

	asIScriptContext *cmpContext = 0;
	bool isNested = false;

	if( mSubTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if ( cmpContext )
		{
			if( cmpContext->GetEngine() == mObjectType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = mObjectType->GetEngine()->CreateContext();
		}
	}

	// Find the matching element
	cgInt ret = -1;
	cgUInt32 size = getSize();
	for ( cgUInt32 i = startAt; i < size; ++i )
	{
		// value passed by reference
		if ( equals(at(i), value, cmpContext, cache) )
		{
			ret = (cgInt)i;
			break;
		}
	}

	if ( cmpContext )
	{
		if ( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
	}

	return ret;
}

//-----------------------------------------------------------------------------
//  Name : copy () (Protected)
/// <summary>
/// Copy an object handle or primitive value
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::copy( void * dst, void * src )
{
	memcpy(dst, src, mElementSize);
}

//-----------------------------------------------------------------------------
//  Name : getArrayItemPointer () (Protected)
/// <summary>
/// Return pointer to array item (object handle or primitive value)
/// </summary>
//-----------------------------------------------------------------------------
void * ScriptArray::getArrayItemPointer( cgInt index )
{
	return mBuffer->data + index * mElementSize;
}


//-----------------------------------------------------------------------------
//  Name : getArrayItemPointer () (Protected)
/// <summary>
/// Return pointer to data in buffer (object or primitive)
/// </summary>
//-----------------------------------------------------------------------------
void * ScriptArray::getDataPointer( void * buffer )
{
	if ((mSubTypeId & asTYPEID_MASK_OBJECT) && !(mSubTypeId & asTYPEID_OBJHANDLE) )
	{
		// Real address of object
		return reinterpret_cast<void*>(*(size_t*)buffer);
	}
	else
	{
		// Primitive is just a raw data
		return buffer;
	}
}

//-----------------------------------------------------------------------------
//  Name : sortAsc ()
/// <summary>
/// Sort the contents of the array in ascending order.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::sortAsc()
{
	sort(0, getSize(), true);
}

//-----------------------------------------------------------------------------
//  Name : sortAsc ()
/// <summary>
/// Sort the specified subregion of the array in ascending order.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::sortAsc( cgUInt32 startAt, cgUInt32 count )
{
	sort(startAt, count, true);
}

//-----------------------------------------------------------------------------
//  Name : sortDesc ()
/// <summary>
/// Sort the contents of the array in descending order.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::sortDesc()
{
	sort(0, getSize(), false);
}

//-----------------------------------------------------------------------------
//  Name : sortDesc ()
/// <summary>
/// Sort the specified subregion of the array in descending order.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::sortDesc( cgUInt32 startAt, cgUInt32 count )
{
	sort(startAt, count, false);
}


//-----------------------------------------------------------------------------
//  Name : sort () (Protected)
/// <summary>
/// Internal common sorting method.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::sort( cgUInt32 startAt, cgUInt32 count, bool ascending )
{
	// Subtype isn't primitive and doesn't have opCmp
	ArrayCache *cache = reinterpret_cast<ArrayCache*>(mObjectType->GetUserData(ARRAY_CACHE));
	if ( mSubTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		if( !cache || cache->cmpFunc == 0 )
		{
			asIScriptContext *ctx = asGetActiveContext();
			
			// Throw an exception
			if ( ctx )
			{
				char tmp[512];

				if( cache && cache->cmpFuncReturnCode == asMULTIPLE_FUNCTIONS )
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' has multiple matching opCmp methods", mSubType->GetName());
#else
					sprintf(tmp, "Type '%s' has multiple matching opCmp methods", mSubType->GetName());
#endif
				else
#if defined(_MSC_VER) && _MSC_VER >= 1500 && !defined(__S3E__)
					sprintf_s(tmp, 512, "Type '%s' does not have a matching opCmp method", mSubType->GetName());
#else
					sprintf(tmp, "Type '%s' does not have a matching opCmp method", mSubType->GetName());
#endif

				ctx->SetException(tmp);
			}

			return;
		}
	}

	// No need to sort
	if ( count < 2 )
	    return;

	cgInt start = (cgInt)startAt;
	cgInt end = (cgInt)startAt + count;

	// Check if we could access invalid item while sorting
	if ( start >= (cgInt)mBuffer->elementCount || end > (cgInt)mBuffer->elementCount )
	{
		asIScriptContext *ctx = asGetActiveContext();

		// Throw an exception
		if( ctx )
			ctx->SetException("Index out of bounds");
		return;
	}

	cgByte tmp[16];
	asIScriptContext *cmpContext = 0;
	bool isNested = false;

	if( mSubTypeId & ~asTYPEID_MASK_SEQNBR )
	{
		// Try to reuse the active context
		cmpContext = asGetActiveContext();
		if( cmpContext )
		{
			if( cmpContext->GetEngine() == mObjectType->GetEngine() && cmpContext->PushState() >= 0 )
				isNested = true;
			else
				cmpContext = 0;
		}
		if( cmpContext == 0 )
		{
			// TODO: Ideally this context would be retrieved from a pool, so we don't have to
			//       create a new one everytime. We could keep a context with the array object
			//       but that would consume a lot of resources as each context is quite heavy.
			cmpContext = mObjectType->GetEngine()->CreateContext();
		}
	}

	// Insertion sort
	for ( cgInt i = start + 1; i < end; ++i )
	{
		copy(tmp, getArrayItemPointer(i));

		cgInt j = i - 1;
		while ( j >= start && less(getDataPointer(tmp), at(j), ascending, cmpContext, cache) )
		{
			copy(getArrayItemPointer(j + 1), getArrayItemPointer(j));
			j--;
		}

		copy(getArrayItemPointer(j + 1), tmp);
	}

	if ( cmpContext )
    {
		if( isNested )
		{
			asEContextState state = cmpContext->GetState();
			cmpContext->PopState();
			if( state == asEXECUTION_ABORTED )
				cmpContext->Abort();
		}
		else
			cmpContext->Release();
    }
}

//-----------------------------------------------------------------------------
//  Name : copyBuffer () (Protected)
/// <summary>
/// Duplicate elements from one buffer to the other.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::copyBuffer( ArrayBuffer * dst, ArrayBuffer * src )
{
    asIScriptEngine *engine = mObjectType->GetEngine();
	if ( mSubTypeId & asTYPEID_OBJHANDLE )
	{
		// Copy the references and increase the reference counters
		if ( dst->elementCount > 0 && src->elementCount > 0 )
		{
			cgInt count = dst->elementCount > src->elementCount ? src->elementCount : dst->elementCount;

			void **max = (void**)(dst->data + count * sizeof(void*));
			void **d   = (void**)dst->data;
			void **s   = (void**)src->data;

			for ( ; d < max; d++, s++ )
			{
				void *tmp = *d;
				*d = *s;
				if( *d )
					engine->AddRefScriptObject(*d, mSubType );
				// Release the old ref after incrementing the new to avoid problem incase it is the same ref
				if( tmp )
					engine->ReleaseScriptObject(tmp, mSubType );
			}
		}
	}
	else
	{
		if( dst->elementCount > 0 && src->elementCount > 0 )
		{
			cgInt count = dst->elementCount > src->elementCount ? src->elementCount : dst->elementCount;
			if ( mSubTypeId & asTYPEID_MASK_OBJECT )
			{
				// Call the assignment operator on all of the objects
				void **max = (void**)(dst->data + count * sizeof(void*));
				void **d   = (void**)dst->data;
				void **s   = (void**)src->data;
				for( ; d < max; d++, s++ )
					engine->AssignScriptObject(*d, *s, mSubType);
			}
			else
			{
				// Primitives are copied byte for byte
				memcpy(dst->data, src->data, count*mElementSize);
			}
		}
	}
}

//-----------------------------------------------------------------------------
//  Name : cacheDetails() (Protected)
/// <summary>
/// Pre-cache information about the array/
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::cacheDetails()
{
	mSubTypeId = mObjectType->GetSubTypeId();
    mSubType = mObjectType->GetEngine()->GetObjectTypeById(mSubTypeId);
    if ( mSubType )
        mSubType->AddRef();

	// Check if it is an array of objects. Only for these do we need to cache anything
	// Type ids for primitives and enums only has the sequence number part
	if( !(mSubTypeId & ~asTYPEID_MASK_SEQNBR) )
		return;

	// The opCmp and opEquals methods are cached because the searching for the
	// methods is quite time consuming if a lot of array objects are created.

	// First check if a cache already exists for this array type
	ArrayCache *cache = reinterpret_cast<ArrayCache*>(mObjectType->GetUserData(ARRAY_CACHE));
	if ( cache )
        return;

	// We need to make sure the cache is created only once, even
	// if multiple threads reach the same point at the same time
	asAcquireExclusiveLock();

	// Now that we got the lock, we need to check again to make sure the
	// cache wasn't created while we were waiting for the lock
	cache = reinterpret_cast<ArrayCache*>(mObjectType->GetUserData(ARRAY_CACHE));
	if ( cache )
	{
		asReleaseExclusiveLock();
		return;
	}

	// Create the cache
	cache = new ArrayCache();
	memset(cache, 0, sizeof(ArrayCache));

	// If the sub type is a handle to const, then the methods must be const too
	bool mustBeConst = (mSubTypeId & asTYPEID_HANDLETOCONST) ? true : false;
	if ( mSubType )
	{
		for ( cgUInt32 i = 0; i < mSubType->GetMethodCount(); i++ )
		{
			asIScriptFunction *func = mSubType->GetMethodByIndex(i);

			if( func->GetParamCount() == 1 && (!mustBeConst || func->IsReadOnly()) )
			{
				asDWORD flags = 0;
				cgInt returnTypeId = func->GetReturnTypeId(&flags);

				// The method must not return a reference
				if( flags != asTM_NONE )
					continue;

				// opCmp returns an int and opEquals returns a bool
				bool isCmp = false, isEq = false;
				if( returnTypeId == asTYPEID_INT32 && strcmp(func->GetName(), "opCmp") == 0 )
					isCmp = true;
				if( returnTypeId == asTYPEID_BOOL && strcmp(func->GetName(), "opEquals") == 0 )
					isEq = true;

				if( !isCmp && !isEq )
					continue;

				// The parameter must either be a reference to the subtype or a handle to the subtype
				cgInt paramTypeId = func->GetParamTypeId(0, &flags);

				if( (paramTypeId & ~(asTYPEID_OBJHANDLE|asTYPEID_HANDLETOCONST)) != (mSubTypeId &  ~(asTYPEID_OBJHANDLE|asTYPEID_HANDLETOCONST)) )
					continue;

				if( (flags & asTM_INREF) )
				{
					if( (paramTypeId & asTYPEID_OBJHANDLE) || (mustBeConst && !(flags & asTM_CONST)) )
						continue;
				}
				else if( paramTypeId & asTYPEID_OBJHANDLE )
				{
					if( mustBeConst && !(paramTypeId & asTYPEID_HANDLETOCONST) )
						continue;
				}
				else
					continue;

				if( isCmp )
				{
					if( cache->cmpFunc || cache->cmpFuncReturnCode )
					{
						cache->cmpFunc = 0;
						cache->cmpFuncReturnCode = asMULTIPLE_FUNCTIONS;
					}
					else
						cache->cmpFunc = func;
				}
				else if( isEq )
				{
					if( cache->eqFunc || cache->eqFuncReturnCode )
					{
						cache->eqFunc = 0;
						cache->eqFuncReturnCode = asMULTIPLE_FUNCTIONS;
					}
					else
						cache->eqFunc = func;
				}
			}
		}
	}

	if( cache->eqFunc == 0 && cache->eqFuncReturnCode == 0 )
		cache->eqFuncReturnCode = asNO_FUNCTION;
	if( cache->cmpFunc == 0 && cache->cmpFuncReturnCode == 0 )
		cache->cmpFuncReturnCode = asNO_FUNCTION;

	// Set the user data only at the end so others that retrieve it will know it is complete
	mObjectType->SetUserData(cache, ARRAY_CACHE);

	asReleaseExclusiveLock();
}

//-----------------------------------------------------------------------------
//  Name : addRef ()
/// <summary>
/// Add reference to script array object.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::addRef()
{
    // Clear the GC flag then increase the counter
	mGCFlag = false;
    asAtomicInc(mRefCount);
}

//-----------------------------------------------------------------------------
//  Name : release ()
/// <summary>
/// Release reference to script array object.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::release()
{
    // Now do the actual releasing (clearing the flag set by GC)
	mGCFlag = false;
    if ( asAtomicDec(mRefCount) == 0 )
        delete this;
}

//-----------------------------------------------------------------------------
//  Name : setGCFlag ()
/// <summary>
/// Used to set the state of the garbage collection flag to 'true'.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::setGCFlag()
{
	mGCFlag = true;
}

//-----------------------------------------------------------------------------
//  Name : getGCFlag ()
/// <summary>
/// Used to get the current state of the garbage collection flag.
/// </summary>
//-----------------------------------------------------------------------------
bool ScriptArray::getGCFlag()
{
	return mGCFlag;
}

//-----------------------------------------------------------------------------
//  Name : getRefCount ()
/// <summary>
/// Retrieve the current number of references to this object.
/// </summary>
//-----------------------------------------------------------------------------
cgInt ScriptArray::getRefCount()
{
	return mRefCount;
}

//-----------------------------------------------------------------------------
//  Name : releaseAllHandles ()
/// <summary>
/// Garbage collector requests that we release all handles we own.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::releaseAllHandles( asIScriptEngine * engine )
{
	// Resizing to zero will release everything
	resize(0);
}

//-----------------------------------------------------------------------------
//  Name : enumReferences ()
/// <summary>
/// Garbage collector requests that we enumerate the handles we hold.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::enumReferences( asIScriptEngine *engine )
{
	// If the array is holding handles, then we need to notify the GC of them
	if ( mSubTypeId & asTYPEID_MASK_OBJECT )
	{
		void **d = (void**)mBuffer->data;
		for( asUINT n = 0; n < mBuffer->elementCount; n++ )
		{
			if ( d[n] )
				engine->GCEnumCallback(d[n]);
		
        } // Next element
	
    } // End if holds objects
}

///////////////////////////////////////////////////////////////////////////////
// BindException Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : BindException () (Constructor)
/// <summary>
/// BindException Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
BindException::BindException( cgInt _nError, const cgDebugSourceInfo & _debugSource )
{
    error      = _nError;
    sourceFile  = _debugSource;
}

//-----------------------------------------------------------------------------
//  Name : getExceptionSource ()
/// <summary>
/// Retrieve the formatted string outlining the source of the exception.
/// </summary>
//-----------------------------------------------------------------------------
cgString BindException::getExceptionSource() const
{
    cgString strSource = _T("Unavailable");
 
    // File available?
    if ( sourceFile.source.empty() == false )
    {
        // Get filename only portion of source
        strSource = cgFileSystem::getFileName( sourceFile.source );

    } // End if file available

    // Build full source string including line
    strSource = cgString::format( _T("%s(%i)"), strSource.c_str(), sourceFile.line );

    // Return the exception source
    return strSource;
}

///////////////////////////////////////////////////////////////////////////////
// ExecuteException Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : ExecuteException () (Constructor)
/// <summary>
/// ExecuteException Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
ExecuteException::ExecuteException( const cgChar * _strErrorDesc, const cgString & _strScriptResource, cgInt _nFileLine )
{
    STRING_CONVERT;
    description        = stringConvertA2CT( _strErrorDesc );
    scriptResource   = _strScriptResource;
    fileLine           = _nFileLine;
}

//-----------------------------------------------------------------------------
//  Name : getExceptionSource ()
/// <summary>
/// Retrieve the formatted string outlining the source of the exception.
/// </summary>
//-----------------------------------------------------------------------------
cgString ExecuteException::getExceptionSource() const
{
    // Build full source string including line
    cgString strSource = cgString::format( _T("%s(%i)"), scriptResource.c_str(), fileLine );

    // Return the exception source
    return strSource;
}

///////////////////////////////////////////////////////////////////////////////
// ObjectSerializer Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : ObjectSerializer () (Constructor)
/// <summary>
/// ObjectSerializer Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
ObjectSerializer::ObjectSerializer( ) :
    mEngine(CG_NULL)
{
}

//-----------------------------------------------------------------------------
//  Name : ~ObjectSerializer () (Destructor)
/// <summary>
/// ObjectSerializer Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
ObjectSerializer::~ObjectSerializer( )
{
    clear();
}

//-----------------------------------------------------------------------------
//  Name : serialize ()
/// <summary>
/// Serialize the specified script object so that it can later be restored.
/// </summary>
//-----------------------------------------------------------------------------
bool ObjectSerializer::serialize( cgScriptObject * object )
{
    // Get type data
    asIScriptObject * scriptObject = object->getInternalObject();
    asIObjectType   * scriptType   = scriptObject->GetObjectType();
    
    // Store information needed to reconstruct.
    mEngine   = scriptObject->GetEngine();
    mTypeName = scriptType->GetName();
    mEngine->AddRef();

    // Build array sub type map.
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("bool[]")] = asTYPEID_BOOL;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("int8[]")] = asTYPEID_INT8;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("uint8[]")] = asTYPEID_UINT8;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("int16[]")] = asTYPEID_INT16;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("uint16[]")] = asTYPEID_UINT16;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("int[]")] = asTYPEID_INT32;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("uint[]")] = asTYPEID_UINT32;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("int64[]")] = asTYPEID_INT64;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("uint64[]")] = asTYPEID_UINT64;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("float[]")] = asTYPEID_FLOAT;
    mArraySubTypeMap[mEngine->GetTypeIdByDecl("double[]")] = asTYPEID_DOUBLE;

    // Serialize the object values.
    return serializeValue( mMembers, scriptObject, scriptType->GetTypeId(), scriptType );
}

//-----------------------------------------------------------------------------
//  Name : deserialize ()
/// <summary>
/// Create a new script object of the same type previously serialized, and
/// reconstruct its members.
/// </summary>
//-----------------------------------------------------------------------------
cgScriptObject * ObjectSerializer::deserialize( cgScript * script, cgUInt32 & result )
{
    STRING_CONVERT;

    // Create an *uninitialized* object instance.
    cgScriptObject * object = script->createObjectInstance( stringConvertA2CT(mTypeName.c_str()), true );
    if ( !object )
        return CG_NULL;

    // Attempt to deserialize.
    asIScriptObject * scriptObject = object->getInternalObject();
    asIObjectType   * scriptType   = scriptObject->GetObjectType();
    result = deserializeValue( mMembers, scriptObject, scriptType->GetTypeId(), scriptType, script );

    // If it failed, make sure the object is released.
    if ( result & DeserializeResult::Failed )
    {
        object->release();
        return CG_NULL;
    
    } // End if failed

    // Return the new object.
    return object;
}

//-----------------------------------------------------------------------------
//  Name : isEmpty ()
/// <summary>
/// Currently stores serialized data?
/// </summary>
//-----------------------------------------------------------------------------
bool ObjectSerializer::isEmpty( ) const
{
    return (mEngine == CG_NULL);
}

//-----------------------------------------------------------------------------
//  Name : clear ()
/// <summary>
/// Clear out serialized data.
/// </summary>
//-----------------------------------------------------------------------------
void ObjectSerializer::clear( )
{
    // Clear out value data.
    clear( mMembers );

    // Clear other variables.
    mTypeName.clear();
    mArraySubTypeMap.clear();

    // Release our reference to the engine.
    if ( mEngine )
        mEngine->Release();
    mEngine = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : clear () (Private, Recursive)
/// <summary>
/// Clear out serialized data.
/// </summary>
//-----------------------------------------------------------------------------
void ObjectSerializer::clear( SerializedValue::Map & values )
{
    SerializedValue::Map::iterator itValue;
    for ( itValue = values.begin(); itValue != values.end(); ++itValue )
    {
        SerializedValue & value = itValue->second;
        if ( value.isArray )
        {
            if ( value.reference )
                free( value.reference );
        
        } // End if is array
        else if ( value.typeId & asTYPEID_OBJHANDLE )
        {
            if ( value.reference )
                mEngine->ReleaseScriptObject( value.reference, value.type );

            // Release any children.
            clear( value.members );

        } // End if object handle

        // Delete value data.
        if ( value.value )
            free( value.value );

        // We're done with the type data.
        if ( value.type )
            value.type->Release();

    } // Next Member
    values.clear();
}

//-----------------------------------------------------------------------------
//  Name : deserializeValue () (Private, Recursive)
/// <summary>
/// Recursively restore member data.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ObjectSerializer::deserializeValue( SerializedValue::Map & values, asIScriptObject * object, cgInt typeId, asIObjectType * type, cgScript * script )
{
    cgUInt32 result = DeserializeResult::Success;

    // Process type members.
    asUINT memberCount  = type->GetPropertyCount();
    for ( asUINT i = 0; i < memberCount; ++i )
    {
        // Get basic information about the member.
        cgInt memberTypeId;
        const char * memberName;
        bool isPrivate, isReference;
        type->GetProperty( i, &memberName, &memberTypeId, &isPrivate, 0, &isReference, 0);

        // Attempt to find the matching value.
        SerializedValue::Map::iterator itValue = values.find( memberName );
        if ( itValue == values.end() )
        {
            // We didn't serialize it, ignore but inform the caller.
            result |= DeserializeResult::UninitializedMembers;
            continue;
        
        } // End if not found

        // Extract value.
        result |= deserializeValue( itValue->second, memberTypeId, mEngine->GetObjectTypeById( memberTypeId ), object->GetAddressOfProperty(i), script );
        if ( result & DeserializeResult::Failed )
            return result;
        
    } // Next value

    // Success!
    return result;
}

//-----------------------------------------------------------------------------
//  Name : deserializeValue () (Private, Recursive)
/// <summary>
/// Recursively restore member data.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ObjectSerializer::deserializeValue( SerializedValue::Array & values, ScriptArray * scriptArray, cgInt typeId, asIObjectType * type, cgScript * script )
{
    cgUInt32 result = DeserializeResult::Success;

    // Get array element description.
    cgUInt32 elementCount = values.size();
    cgInt elementTypeId = scriptArray->getElementTypeId();
    asIObjectType * elementType = mEngine->GetObjectTypeById( elementTypeId );

    // Size the output array.
    scriptArray->resize( elementCount );

    // Process array members.
    for ( cgUInt32 i = 0; i < elementCount; ++i )
    {
        // Restore value
        result |= deserializeValue( values[i], elementTypeId, elementType, scriptArray->at(i), script );
        if ( result & DeserializeResult::Failed )
            return result;
        
    } // Next value

    // Success!
    return result;
}

//-----------------------------------------------------------------------------
//  Name : deserializeValue () (Private, Recursive)
/// <summary>
/// Recursively restore member data.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ObjectSerializer::deserializeValue( SerializedValue & value, cgInt destTypeId, asIObjectType * destType, void * destObject, cgScript * script )
{
    if ( value.typeId & asTYPEID_OBJHANDLE )
    {
        // Was this a script object?
        if ( value.typeId & asTYPEID_SCRIPTOBJECT )
        {
            // We can only do this if the destination has the same type, and is
            // also a handle to a script object object.
            if ( !destType || !(destTypeId & asTYPEID_SCRIPTOBJECT) || !(destTypeId & asTYPEID_OBJHANDLE) || (value.typeName != destType->GetName()) )
            {
                // We didn't serialize it, ignore but inform the caller.
                return DeserializeResult::UninitializedMembers;
            
            } // End if can't restore

            // If a bridge was available, reconnect to the object.
            if ( value.hasBridge )
            {
                asIScriptObject * originalObject = (asIScriptObject*)value.reference;
                ObjectSerializerBridge * newObjectBridge = (ObjectSerializerBridge*)originalObject->GetUserData(SERIALIZE_OBJECT_BRIDGE);
                cgScriptObject * newObject = (newObjectBridge) ? (cgScriptObject*)newObjectBridge->getData() : CG_NULL;

                // AddRef() the new object before we pass it back in (if there is one).
                asIScriptObject * newObjectInternal = (newObject) ? newObject->getInternalObject() : CG_NULL;
                if ( newObjectInternal )
                    mEngine->AddRefScriptObject( newObjectInternal, value.type );

                // Assign to the output.
                *(void**)destObject = newObjectInternal;
            
            } // End if has bridge
            else
            {
                // No bridge is available, create a new (uninitialized) instance of the type?
                if ( value.wasInitialized )
                {
                    STRING_CONVERT;
                    cgUInt32 result = DeserializeResult::Success;
                    cgScriptObject * newObjectBridge = script->createObjectInstance( stringConvertA2CT(value.typeName.c_str()), false );
                    asIScriptObject * newObject = (newObjectBridge) ? newObjectBridge->getInternalObject() : CG_NULL;
                    if ( newObject )
                    {
                        // Recurse
                        result |= deserializeValue( value.members, newObject, destTypeId, destType, script );
                        if ( result & DeserializeResult::Failed )
                        {
                            newObjectBridge->release();
                            return result;
                        
                        } // End if failed

                        // AddRef() before we pass it back in.
                        mEngine->AddRefScriptObject( newObject, destType );

                        // Assign to the member.
                        *(void**)destObject = newObject;
                    
                    } // End if object created
                    else
                    {
                        // We couldn't recreate it, ignore but inform the caller.
                        result |= DeserializeResult::UninitializedMembers;
                    
                    } // End if no object

                    // We're done with our temporary bridge.
                    if ( newObjectBridge )
                        newObjectBridge->release();
                    return result;

                } // End if was initialized

            } // End if no bridge

        } // End if was script object
        else
        {
            // We can only do this if the destination has the same type, and is
            // also a handle to an application object.
            if ( !destType || (destTypeId & asTYPEID_SCRIPTOBJECT) || !(destTypeId & asTYPEID_OBJHANDLE) || (value.typeName != destType->GetName()) )
            {
                // We didn't serialize it, ignore but inform the caller.
                return DeserializeResult::UninitializedMembers;
            
            } // End if can't restore

            // AddRef() before we pass it back in.
            if ( value.reference )
                mEngine->AddRefScriptObject( value.reference, value.type );

            // Copy back in.
            *(void**)destObject = value.reference;

        } // End if app object
        
    } // End if object handle
    else if( value.typeId & asTYPEID_SCRIPTOBJECT )
    {
        // We can only do this if the destination has the same type, and is
        // also a script object.
        if ( !destType || !(destTypeId & asTYPEID_SCRIPTOBJECT) || (destTypeId & asTYPEID_OBJHANDLE) || (value.typeName != destType->GetName()) )
        {
            // We didn't serialize it, ignore but inform the caller.
            return DeserializeResult::UninitializedMembers;
        
        } // End if can't restore

        // Recurse.
        cgUInt32 result = deserializeValue( value.members, (asIScriptObject*)destObject, destTypeId, destType, script );
        if ( result & DeserializeResult::Failed )
            return result;
        
    } // End if script object
    else
    {
        // Restore the value
        if ( value.isArray )
        {
            // Process as required.
            if ( value.reference )
            {
                cgUInt32 arrayLength = *(cgUInt32*)value.reference;
                cgUInt32 elementSize = *(((cgUInt32*)value.reference)+1);
                void * arrayBuffer = ((cgByte*)value.reference) + sizeof(cgUInt32) * 2;
                
                // Initialize the array
                cgArrayBase & internalArray = *((cgArrayBase*)destObject);
                internalArray.alloc_uninitialized( arrayLength, elementSize );
                if ( arrayLength )
                    memcpy( internalArray.buffer(), arrayBuffer, arrayLength * elementSize );       

            } // End if known type
            else
            {
                // This is a templated array type. Process elements!
                return deserializeValue( value.elements, (ScriptArray*)destObject, destTypeId, destType, script );

            } // End if unknown type

        } // End if isArray
        else if ( value.typeName == "String" )
        {
            if ( value.value )
                *((cgString*)destObject) = cgString( (cgTChar*)value.value, value.valueSize / sizeof(cgTChar) );

        } // End if string
        else
        {
            // Copy value data over.
            if ( value.value )
                memcpy( destObject, value.value, value.valueSize );
        
        } // End if not array

    } // End if other

    // Success!
    return DeserializeResult::Success;
}

//-----------------------------------------------------------------------------
//  Name : serializeValue () (Private, Recursive)
/// <summary>
/// Recursively store member data for later reconstruction.
/// </summary>
//-----------------------------------------------------------------------------
bool ObjectSerializer::serializeValue( SerializedValue::Map & values, asIScriptObject * object, cgInt typeId, asIObjectType * type )
{
    // Process type members.
    asUINT memberCount  = type->GetPropertyCount();
    for ( asUINT i = 0; i < memberCount; ++i )
    {
        // Get basic information about the member.
        cgInt memberTypeId;
        const char * memberName;
        bool isPrivate, isReference;
        type->GetProperty( i, &memberName, &memberTypeId, &isPrivate, 0, &isReference, 0);
        
        // Record information about this member.
        SerializedValue & value = values[memberName];
        value.name          = memberName;
        value.typeId        = memberTypeId;
        value.type          = mEngine->GetObjectTypeById( value.typeId );
        value.isPrivate     = isPrivate;
        value.isReference   = isReference;

        // Interpret
        serializeValue( value, object->GetAddressOfProperty(i) );
    
    } // Next member

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : serializeValue () (Private, Recursive)
/// <summary>
/// Recursively store member data for later reconstruction.
/// </summary>
//-----------------------------------------------------------------------------
bool ObjectSerializer::serializeValue( SerializedValue::Array & values, ScriptArray * scriptArray, cgInt typeId, asIObjectType * type )
{
    // Get array element description.
    cgUInt32 elementCount = scriptArray->getSize();
    cgInt elementTypeId = scriptArray->getElementTypeId();
    asIObjectType * elementType = mEngine->GetObjectTypeById( elementTypeId );

    // Make enough space to back up each element.
    values.resize( elementCount );
    
    // Process elements
    for ( cgUInt32 i = 0; i < elementCount; ++i )
    {
        // Record information about this member.
        SerializedValue & value = values[i];
        value.typeId        = elementTypeId;
        value.type          = elementType;
        value.isPrivate     = false;
        value.isReference   = false;

        // Interpret
        serializeValue( value, scriptArray->at(i) );
    
    } // Next member

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : serializeValue () (Private, Recursive)
/// <summary>
/// Populate the specified value structure based on the details of the
/// specified object.
/// </summary>
//-----------------------------------------------------------------------------
bool ObjectSerializer::serializeValue( SerializedValue & value, void * object )
{
    value.reference     = CG_NULL;
    value.value         = CG_NULL;
    value.valueSize     = 0;
    value.subTypeId     = 0;
    value.hasBridge     = false;
    value.wasInitialized= false;
    value.isArray       = false;

    // AddRef() type if we can.
    if ( value.type )
    {
        value.type->AddRef();
        value.typeName = value.type->GetName();
    
    } // End if has type

    // Extract value.
    if ( value.typeId & asTYPEID_OBJHANDLE )
    {
        // Get the object value
        if ( object && *(void**)object )
        {
            bool reconnect = true;
            value.wasInitialized= true;

            // If this is a script object, we may be able to simply
            // reconnect to it if it has a serializer bridge. If not
            // we need to serialize its full state.
            if ( value.typeId & asTYPEID_SCRIPTOBJECT )
            {
                asIScriptObject * memberObject = *(asIScriptObject**)object;
                if ( !memberObject->GetUserData( SERIALIZE_OBJECT_BRIDGE ) )
                {
                    reconnect = false;
                    value.hasBridge = false;

                    // No bridge available, recurse.
                    bool result = serializeValue( value.members, memberObject, value.typeId, value.type );
                    if ( !result )
                        return false;

                } // End if no bridge
                else
                {
                    value.hasBridge = true;
                
                } // End if has bridge

            } // End if script object

            // Can we reconnect to this reference?
            if ( reconnect )
            {
                // AddRef() so it doesn't get released.
                mEngine->AddRefScriptObject( *(void**)object, value.type );

                // Store the reference.
                value.reference = *(void**)object;
            
            } // End if set
        
        } // End if app object
    
    } // End if object handle
    else if ( value.typeId & asTYPEID_SCRIPTOBJECT )
    {
        // Recurse.
        if ( object )
        {
            value.wasInitialized = true;
            bool result = serializeValue( value.members, (asIScriptObject*)object, value.typeId, value.type );
            if ( !result )
                return false;
        
        } // End if !null

    } // End if script object
    else
    {
        value.wasInitialized = true;
        if ( value.typeName == "array" )
        {
            value.isArray = true;
            value.subTypeId = value.type->GetSubTypeId();
            
            // If it has no sub type (i.e. not a template type), it must be one of our internal types.
            if ( value.subTypeId == asERROR )
            {
                cgArrayBase & internalArray = *((cgArrayBase*)object);
                value.reference = malloc( internalArray.size() * internalArray.element_size() + (sizeof(cgUInt32) * 2) );
                *((cgUInt32*)value.reference) = (cgUInt32)internalArray.size();
                *(((cgUInt32*)value.reference) + 1) = (cgUInt32)internalArray.element_size();
                if ( !internalArray.empty() )
                    memcpy( ((cgUInt32*)value.reference) + 2, internalArray.buffer(), internalArray.size() * internalArray.element_size() );

            } // End if known type
            else
            {
                // This is a templated array type. Process elements!
                bool result = serializeValue( value.elements, (ScriptArray*)object, value.typeId, value.type );
                if ( !result )
                    return false;

            } // End if unknown type

        } // End if is array
        else if ( value.typeName == "String" )
        {
            value.valueSize = ((cgString*)object)->size() * sizeof(cgTChar);
            if ( value.valueSize )
            {
                value.value = malloc( value.valueSize );
                memcpy( value.value, &((cgString*)object)->at(0), value.valueSize );
            
            } // End if has size

        } // End if string
        else
        {
            if ( !value.type )
            {
                switch ( value.typeId )
                {
                    case asTYPEID_BOOL:
                    case asTYPEID_INT8:
                    case asTYPEID_UINT8:
                        value.valueSize = 1;
                        break;
                    case asTYPEID_INT16:
                    case asTYPEID_UINT16:
                        value.valueSize = 2;
                        break;
                    case asTYPEID_INT32:
                    case asTYPEID_UINT32:
                    case asTYPEID_FLOAT:
                        value.valueSize = 4;
                        break;
                    
                    case asTYPEID_INT64:
                    case asTYPEID_UINT64:
                    case asTYPEID_DOUBLE:
                        value.valueSize = 8;
                        break;

                } // End switch type

            } // End if unknown type
            else
            {
                // Get the size from the type.
                value.valueSize = value.type->GetSize();
            
            } // End if known type

            // Copy the data
            if ( value.valueSize )
            {
                value.value = malloc( value.valueSize );
                memcpy( value.value, object, value.valueSize );
            
            } // End if has data

        } // End if not array

    } // End if other

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// Global Function Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : ObjectSerializerBridge () (Constructor)
/// <summary>
/// ObjectSerializerBridge Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
ObjectSerializerBridge::ObjectSerializerBridge( ) :
    mRefCount(1), mData(0)
{
}

//-----------------------------------------------------------------------------
//  Name : setData ()
/// <summary>
/// Store a reference to the object to be used when the serializer needs to
/// bridge between a prior object any replacement object.
/// </summary>
//-----------------------------------------------------------------------------
void ObjectSerializerBridge::setData( void * data )
{
    mData = data;
}

//-----------------------------------------------------------------------------
//  Name : getData ()
/// <summary>
/// Retrieve the reference to the object to be used when the serializer needs 
/// to bridge between a prior object any replacement object.
/// </summary>
//-----------------------------------------------------------------------------
void * ObjectSerializerBridge::getData( )
{
    return mData;
}

//-----------------------------------------------------------------------------
//  Name : addRef ()
/// <summary>
/// Increment internal reference counter.
/// </summary>
//-----------------------------------------------------------------------------
void ObjectSerializerBridge::addRef( )
{
    // ToDo: Make atomic
    ++mRefCount;
}

//-----------------------------------------------------------------------------
//  Name : release ()
/// <summary>
/// Decrement internal reference counter and destroy the object when all 
/// references are released.
/// </summary>
//-----------------------------------------------------------------------------
void ObjectSerializerBridge::release( )
{
    // ToDo: make atomic
    if ( --mRefCount <= 0 )
        delete this;
}

///////////////////////////////////////////////////////////////////////////////
// Global Function Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : getResultName ()
/// <summary>
/// Retrieve the string name representation of the result code.
/// </summary>
//-----------------------------------------------------------------------------
cgString cgScriptInterop::Utils::getResultName( cgInt nResultCode )
{
    switch( nResultCode )
    {
        case asSUCCESS:
            return _T("asSUCCESS");
        case asERROR:
            return _T("asERROR");
        case asCONTEXT_ACTIVE:
            return _T("asCONTEXT_ACTIVE");
        case asCONTEXT_NOT_FINISHED:
            return _T("asCONTEXT_NOT_FINISHED");
        case asCONTEXT_NOT_PREPARED:
            return _T("asCONTEXT_NOT_PREPARED");
        case asINVALID_ARG:
            return _T("asINVALID_ARG");
        case asNO_FUNCTION:
            return _T("asNO_FUNCTION");
        case asNOT_SUPPORTED:
            return _T("asNOT_SUPPORTED");
        case asINVALID_NAME:
            return _T("asINVALID_NAME");
        case asNAME_TAKEN:
            return _T("asNAME_TAKEN");
        case asINVALID_DECLARATION:
            return _T("asINVALID_DECLARATION");
        case asINVALID_OBJECT:
            return _T("asINVALID_OBJECT");
        case asINVALID_TYPE:
            return _T("asINVALID_TYPE");
        case asALREADY_REGISTERED:
            return _T("asALREADY_REGISTERED");
        case asMULTIPLE_FUNCTIONS:
            return _T("asMULTIPLE_FUNCTIONS");
        case asNO_MODULE:
            return _T("asNO_MODULE");
        case asNO_GLOBAL_VAR:
            return _T("asNO_GLOBAL_VAR");
        case asINVALID_CONFIGURATION:
            return _T("asINVALID_CONFIGURATION");
        case asINVALID_INTERFACE:
            return _T("asINVALID_INTERFACE");
        case asCANT_BIND_ALL_FUNCTIONS:
            return _T("asCANT_BIND_ALL_FUNCTIONS");
        case asLOWER_ARRAY_DIMENSION_NOT_REGISTERED:
            return _T("asLOWER_ARRAY_DIMENSION_NOT_REGISTERED");
        case asWRONG_CONFIG_GROUP:
            return _T("asWRONG_CONFIG_GROUP");
        case asCONFIG_GROUP_IS_IN_USE:
            return _T("asCONFIG_GROUP_IS_IN_USE");
        case asILLEGAL_BEHAVIOUR_FOR_TYPE:
            return _T("asILLEGAL_BEHAVIOUR_FOR_TYPE");
        case asWRONG_CALLING_CONV:
            return _T("asWRONG_CALLING_CONV");
        case asBUILD_IN_PROGRESS:
            return _T("asBUILD_IN_PROGRESS");
        case asINIT_GLOBAL_VARS_FAILED:
            return _T("asINIT_GLOBAL_VARS_FAILED");
        default:
            return _T("Unknown Result");
    
    } // End Switch nResultCode
}