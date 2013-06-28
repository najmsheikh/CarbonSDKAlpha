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

//-----------------------------------------------------------------------------
// Namespace Promotion
//-----------------------------------------------------------------------------
using namespace cgScriptInterop::Types;
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
ScriptArray::ScriptArray( cgUInt32 nLength, asIObjectType * pObjectType )
{
    // Initialize variables to sensible defaults.
    mRefCount     = 1;    // IMPORTANT!
    mObjectType   = pObjectType;
    mGCFlag       = false;
    pObjectType->AddRef();

    // Determine element size
    mSubTypeId = pObjectType->GetSubTypeId( );
    if ( mSubTypeId & asTYPEID_MASK_OBJECT )
        mElementSize = sizeof(size_t);
    else
        mElementSize = pObjectType->GetEngine()->GetSizeOfPrimitiveType( mSubTypeId );

    // Storing handles?
    mIsArrayOfHandles = (mSubTypeId & asTYPEID_OBJHANDLE) ? true : false;

    // Allocate storage
    createBuffer( &mBuffer, nLength );

    // Notify the GC of the successful creation
	if ( pObjectType->GetFlags() & asOBJ_GC )
		pObjectType->GetEngine()->NotifyGarbageCollectorOfNewObject(this, pObjectType);
}

//-----------------------------------------------------------------------------
//  Name : ~ScriptArray () (Destructor)
/// <summary>
/// ScriptArray Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray::~ScriptArray()
{
    if ( mBuffer != CG_NULL )
        deleteBuffer( mBuffer );
    mBuffer = CG_NULL;
    
    if ( mObjectType != CG_NULL )
        mObjectType->Release();
}

//-----------------------------------------------------------------------------
//  Name : factory () (Static)
/// <summary>
/// Construct a new instance of the ScriptArray class.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray * ScriptArray::factory( asIObjectType * pObjectType )
{
    return new ScriptArray(0, pObjectType);
}

//-----------------------------------------------------------------------------
//  Name : factory () (Static)
/// <summary>
/// Construct a new instance of the ScriptArray class with the specified
/// length.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray * ScriptArray::factory( asIObjectType * pObjectType, cgUInt32 nLength )
{
    return new ScriptArray( nLength, pObjectType );
}

//-----------------------------------------------------------------------------
//  Name : listFactory () (Static)
/// <summary>
/// Construct a new instance of the ScriptArray class with the specified
/// length used exclusively for initialization lists.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray * ScriptArray::listFactory( asIObjectType * pObjectType, cgUInt32 nLength)
{
	ScriptArray * a = new ScriptArray( nLength, pObjectType );

	// It's possible the constructor raised a script exception, in which case we 
	// need to free the memory and return null instead, else we get a memory leak.
	asIScriptContext * ctx = asGetActiveContext();
	if ( ctx && ctx->GetState() == asEXECUTION_EXCEPTION )
	{
		delete a;
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

    // Register the factory that will be used for initialization lists
	BINDSUCCESS( pEngine->RegisterObjectBehaviour("array<T>", asBEHAVE_LIST_FACTORY, "array<T>@ f(int&in, uint)", asFUNCTIONPR(listFactory, (asIObjectType*, cgUInt32), ScriptArray*), asCALL_CDECL) );

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
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "T &opIndex(uint)", asMETHOD(ScriptArray, at), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "const T &opIndex(uint) const", asMETHOD(ScriptArray, at), asCALL_THISCALL) );
    
    // The assignment operator
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "array<T> &opAssign(const array<T>&in)", asMETHOD(ScriptArray, operator=), asCALL_THISCALL) );

    // Other methods
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "uint length() const", asMETHOD(ScriptArray, getSize), asCALL_THISCALL) );
    BINDSUCCESS( pEngine->RegisterObjectMethod("array<T>", "void resize(uint)", asMETHOD(ScriptArray, resize), asCALL_THISCALL) );
}

//-----------------------------------------------------------------------------
//  Name : operator= ()
/// <summary>
/// Assignment operator for this class.
/// </summary>
//-----------------------------------------------------------------------------
ScriptArray &ScriptArray::operator=( const ScriptArray & Other )
{
    // Only perform the copy if the array types are the same
    if ( &Other != this && Other.getArrayObjectType() == getArrayObjectType() )
    {
        if ( mBuffer != CG_NULL )
        {
            deleteBuffer( mBuffer );
            mBuffer = CG_NULL;
        
        } // End if allocated

        // Copy all elements from the other array
        createBuffer( &mBuffer, Other.mBuffer->elementCount );
        copyBuffer( mBuffer, Other.mBuffer);
    
    } // End if matching types

    return *this;
}

//-----------------------------------------------------------------------------
//  Name : getSize ()
/// <summary>
/// Retrieve the number of elements stored in this array.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 ScriptArray::getSize()
{
    return mBuffer->elementCount;
}

//-----------------------------------------------------------------------------
//  Name : resize ()
/// <summary>
/// Resize the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::resize( cgUInt32 nElementCount )
{
    // Is this a no-op?
    if ( nElementCount == mBuffer->elementCount )
        return;

    // Allocate new buffer of the correct size.
    ArrayBuffer * pNewBuffer;
    int nTypeId = mObjectType->GetSubTypeId();
    if ( nTypeId & asTYPEID_MASK_OBJECT )
    {
        pNewBuffer = (ArrayBuffer*)new cgByte[sizeof(ArrayBuffer)-1+sizeof(void*)*nElementCount];
        pNewBuffer->elementCount = nElementCount;

        // Copy the elements from the old buffer
        int c = (nElementCount > mBuffer->elementCount) ? mBuffer->elementCount : nElementCount;
        cgUInt32 **ppDest = (cgUInt32**)pNewBuffer->data;
        cgUInt32 **ppSrc  = (cgUInt32**)mBuffer->data;
        for ( int n = 0; n < c; n++ )
            ppDest[n] = ppSrc[n];

        // Construct new / destruct old elements
        if ( nElementCount > mBuffer->elementCount )
            construct( pNewBuffer, mBuffer->elementCount, nElementCount );
        else if ( nElementCount < mBuffer->elementCount )
            destruct( mBuffer, nElementCount, mBuffer->elementCount );
    
    } // End if object
    else
    {
        // Allocate memory for the buffer
        pNewBuffer = (ArrayBuffer*)new cgByte[sizeof(ArrayBuffer)-1+mElementSize*nElementCount];
        pNewBuffer->elementCount = nElementCount;

        // Copy data into new buffer.
        int c = (nElementCount > mBuffer->elementCount) ? mBuffer->elementCount : nElementCount;
        memcpy( pNewBuffer->data, mBuffer->data, c*mElementSize );
    
    } // End if value

    // Release the old buffer
    delete[] (cgByte*)mBuffer;
    mBuffer = pNewBuffer;
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
int ScriptArray::getArrayTypeId() const
{
    return mObjectType->GetTypeId();
}

//-----------------------------------------------------------------------------
//  Name : getElementTypeId ()
/// <summary>
/// Retrieve the type identifier for the elements contained in this array.
/// </summary>
//-----------------------------------------------------------------------------
int ScriptArray::getElementTypeId() const
{
    return mObjectType->GetSubTypeId();
}

//-----------------------------------------------------------------------------
//  Name : at ()
/// <summary>
/// Retrieve a point to the array element, or CG_NULL if out of bounds.
/// </summary>
//-----------------------------------------------------------------------------
void * ScriptArray::at( cgUInt32 nIndex )
{
    if ( nIndex >= mBuffer->elementCount )
    {
        // If this is called from a script we raise a script exception.
        asIScriptContext *ctx = asGetActiveContext();
        if ( ctx != CG_NULL  )
            ctx->SetException("Index out of bounds");
        return 0;
    
    } // End if out of bounds
    else
    {
        int nTypeId = mObjectType->GetSubTypeId();
        if ( (nTypeId & asTYPEID_MASK_OBJECT) && !mIsArrayOfHandles )
            return (void*)((size_t*)mBuffer->data)[nIndex];
        else
            return mBuffer->data + mElementSize * nIndex;
    
    } // End if in bounds
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
		mObjectType->GetEngine()->AssignScriptObject(ptr, value, mSubTypeId);
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
//  Name : createBuffer () (Protected)
/// <summary>
/// Allocate the array's internal buffer.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::createBuffer( ArrayBuffer **ppBuffer, cgUInt32 nElementCount )
{
    int nTypeId = mObjectType->GetSubTypeId();
    if ( nTypeId & asTYPEID_MASK_OBJECT )
    {
        *ppBuffer = (ArrayBuffer*)new cgByte[sizeof(ArrayBuffer)-1+sizeof(void*)*nElementCount];
        (*ppBuffer)->elementCount = nElementCount;
    
    } // End if object type
    else
    {
        *ppBuffer = (ArrayBuffer*)new cgByte[sizeof(ArrayBuffer)-1+mElementSize*nElementCount];
        (*ppBuffer)->elementCount = nElementCount;
    
    } // End if value type

    // Construct elements.
    construct(*ppBuffer, 0, nElementCount);
}

//-----------------------------------------------------------------------------
//  Name : deleteBuffer () (Protected)
/// <summary>
/// Destroy the array's internal buffer.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::deleteBuffer( ArrayBuffer *pBuffer )
{
    // Destruct elements.
    destruct( pBuffer, 0, pBuffer->elementCount );

    // Free the buffer
    delete[] (cgByte*)pBuffer;
}

//-----------------------------------------------------------------------------
//  Name : construct () (Protected)
/// <summary>
/// Call the constructor for specified elements in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::construct( ArrayBuffer * pBuffer, cgUInt32 nStart, cgUInt32 nEnd )
{
    int nTypeId = mObjectType->GetSubTypeId();
    if ( mIsArrayOfHandles )
    {
        // Set all object handles to null
        cgUInt32 *pDest = (cgUInt32*)(pBuffer->data + nStart * sizeof(void*));
        memset(pDest, 0, (nEnd-nStart)*sizeof(void*));
    
    } // End if handles
    else if ( nTypeId & asTYPEID_MASK_OBJECT )
    {
        cgUInt32 **ppMax = (cgUInt32**)(pBuffer->data + nEnd * sizeof(void*));
        cgUInt32 **ppDest = (cgUInt32**)(pBuffer->data + nStart * sizeof(void*));

        // Construct each element
        asIScriptEngine *pEngine = mObjectType->GetEngine();
        for( ; ppDest < ppMax; ppDest++ )
            *ppDest = (cgUInt32*)pEngine->CreateScriptObject(nTypeId);
    
    } // End if object
}

//-----------------------------------------------------------------------------
//  Name : destruct () (Protected)
/// <summary>
/// Call the destructor for specified elements in the array.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::destruct( ArrayBuffer * pBuffer, cgUInt32 nStart, cgUInt32 nEnd )
{
    bool bDelete = true;
    int nTypeId  = mObjectType->GetSubTypeId();
    if ( nTypeId & asTYPEID_MASK_OBJECT )
    {
        asIScriptEngine * pEngine = mObjectType->GetEngine();
        cgUInt32 **ppMax = (cgUInt32**)(pBuffer->data + nEnd * sizeof(void*));
        cgUInt32 **ppDest = (cgUInt32**)(pBuffer->data + nStart * sizeof(void*));

        // Release
        for( ; ppDest < ppMax; ppDest++ )
        {
            if ( *ppDest )
                pEngine->ReleaseScriptObject( *ppDest, nTypeId );
        
        } // Next Element
    
    } // End if object
}

//-----------------------------------------------------------------------------
//  Name : copyBuffer () (Protected)
/// <summary>
/// Duplicate elements from one buffer to the other.
/// </summary>
//-----------------------------------------------------------------------------
void ScriptArray::copyBuffer( ArrayBuffer * pDest, ArrayBuffer * pSrc )
{
    asIScriptEngine * pEngine = mObjectType->GetEngine();
    if ( mIsArrayOfHandles )
    {
        // Copy the references and increase the reference counters
        if ( pDest->elementCount > 0 && pSrc->elementCount > 0 )
        {
            int nTypeId = mObjectType->GetSubTypeId();
            int nCount  = (pDest->elementCount > pSrc->elementCount) ? pSrc->elementCount : pDest->elementCount;

            cgUInt32 **ppMax  = (cgUInt32**)(pDest->data + nCount * sizeof(void*));
            cgUInt32 **ppDest = (cgUInt32**)pDest->data;
            cgUInt32 **ppSrc  = (cgUInt32**)pSrc->data;

            // Duplicate handle and then addRef.
            for( ; ppDest < ppMax; ppDest++, ppSrc++ )
            {
                *ppDest = *ppSrc;
                if ( *ppDest )
                    pEngine->AddRefScriptObject( *ppDest, nTypeId );
            
            } // Next element
        
        } // End if anything to copy
    
    } // End if handles
    else
    {
        int nTypeId = mObjectType->GetSubTypeId();
        if ( pDest->elementCount > 0 && pSrc->elementCount > 0 )
        {
            int nCount = (pDest->elementCount > pSrc->elementCount) ? pSrc->elementCount : pDest->elementCount;
            if ( nTypeId & asTYPEID_MASK_OBJECT )
            {
                // Call the assignment operator on all of the objects
                cgUInt32 **ppMax  = (cgUInt32**)(pDest->data + nCount * sizeof(void*));
                cgUInt32 **ppDest = (cgUInt32**)pDest->data;
                cgUInt32 **ppSrc  = (cgUInt32**)pSrc->data;

                // Duplicate objects
                for( ; ppDest < ppMax; ppDest++, ppSrc++ )
                    pEngine->AssignScriptObject(*ppDest, *ppSrc, nTypeId);
            
            } // End if object
            else
            {
                // Primitives are copied byte for byte
                memcpy( pDest->data, pSrc->data, nCount * mElementSize);
            
            } // End if value
        
        } // End if anything to copy

    } // End if !handles
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
    mRefCount++;
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
    if ( --mRefCount == 0 )
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