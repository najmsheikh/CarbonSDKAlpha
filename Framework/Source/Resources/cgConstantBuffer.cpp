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
// File : cgConstantBuffer.cpp                                               //
//                                                                           //
// Desc : Contains classes that represent blocks of shader constants that    //
//        can be applied to the device as a group. These constants most      //
//        often provide information to the shader library in order to        //
//        control their behavior.                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgConstantBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/cgConstantBuffer.h>
#include <Resources/cgResourceManager.h>
#include <Rendering/cgRenderDriver.h>
#include <Resources/cgSurfaceShader.h>
#include <Resources/Platform/cgDX9ConstantBuffer.h>
#include <Resources/Platform/cgDX11ConstantBuffer.h>

//-----------------------------------------------------------------------------
// Module Local Variables
//-----------------------------------------------------------------------------
namespace
{
    const cgTChar IdentifierBeginChars[] = _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_");
    const cgTChar IdentifierChars[]      = _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
    const cgTChar NumericChars[]         = _T("0123456789");
};

///////////////////////////////////////////////////////////////////////////////
// cgConstantBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgConstantBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBuffer::cgConstantBuffer( cgUInt32 nReferenceId, const cgConstantBufferDesc & Desc ) : 
    cgResource( nReferenceId ),
    mDesc( Desc )
{
    // Initialize variables to sensible defaults
    mSystemBuffer  = CG_NULL;
    mLocked        = false;
    mDataUpdated   = false;
    mBoundRegister = -1;
    mLockedBuffer  = CG_NULL;

    // Setup initial cached responses
    mResourceType   = cgResourceType::ConstantBuffer;
}

//-----------------------------------------------------------------------------
//  Name : cgConstantBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBuffer::cgConstantBuffer( cgUInt32 nReferenceId, const cgConstantBufferDesc & Desc, const cgConstantTypeDesc::Array & aTypes ) : 
    cgResource( nReferenceId ),
    mDesc( Desc ), mTypes( aTypes )
{
    // Initialize variables to sensible defaults
    mSystemBuffer  = CG_NULL;
    mLocked        = false;
    mDataUpdated   = false;
    mBoundRegister = -1;
    mLockedBuffer  = CG_NULL;

    // Setup initial cached responses
    mResourceType  = cgResourceType::ConstantBuffer;
}

//-----------------------------------------------------------------------------
//  Name : cgConstantBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBuffer::cgConstantBuffer( cgUInt32 nReferenceId, const cgSurfaceShaderHandle & hShader, const cgString & strBufferName ) : 
    cgResource( nReferenceId ),
    mShader( hShader ),
    mBufferName( strBufferName )
{
    // Initialize variables to sensible defaults
    mSystemBuffer  = CG_NULL;
    mLocked        = false;
    mDataUpdated   = false;
    mBoundRegister = -1;
    mLockedBuffer  = CG_NULL;

    // Setup initial cached responses
    mResourceType   = cgResourceType::ConstantBuffer;
}

//-----------------------------------------------------------------------------
//  Name : ~cgConstantBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBuffer::~cgConstantBuffer( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBuffer * cgConstantBuffer::createInstance( cgUInt32 nReferenceId, const cgConstantBufferDesc & Desc )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9ConstantBuffer( nReferenceId, Desc );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11ConstantBuffer( nReferenceId, Desc );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBuffer * cgConstantBuffer::createInstance( cgUInt32 nReferenceId, const cgConstantBufferDesc & Desc, const cgConstantTypeDesc::Array & aTypes )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9ConstantBuffer( nReferenceId, Desc, aTypes );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11ConstantBuffer( nReferenceId, Desc, aTypes );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBuffer * cgConstantBuffer::createInstance( cgUInt32 nReferenceId, const cgSurfaceShaderHandle & hShader, const cgString & strBufferName )
{
    // Determine which state type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9ConstantBuffer( nReferenceId, hShader, strBufferName );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11ConstantBuffer( nReferenceId, hShader, strBufferName );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgConstantBuffer::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Clear containers.
    mTypes.clear();

    // Release child resources
    mShader.close();

    // Dispose base(s).
    if ( bDisposeBase )
        cgResource::dispose( true );
    else
        mDisposing = false;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType () (Virtual)
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_ConstantBufferResource )
        return true;

    // Supported by base?
    return cgResource::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : releaseConstantBuffer ()
/// <summary>
/// Release the internal constant buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgConstantBuffer::releaseConstantBuffer( )
{
    // Unlock if we are still locked
    unlock();

    // Release system memory buffer area.
    delete []mSystemBuffer;
    mSystemBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createConstantBuffer ()
/// <summary>
/// Create the internal constant buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::createConstantBuffer( )
{
    cgToDo( "Carbon General", "Consider stripping down the types array to only those that are referenced by the buffer (and modifying the description structure accordingly." )

    // If loading from shader, retrieve descriptor.
    if ( mShader.isValid() && !mBufferName.empty() )
    {
        cgSurfaceShader * pShader = mShader.getResource( true );
        if ( !pShader->isLoaded() )
            return false;

        // Retrieve types & descriptor.
        mTypes = pShader->getConstantTypes();
        if ( !pShader->getConstantBufferDesc( mBufferName, mDesc ) )
        {
            cgAppLog::write( cgAppLog::Error, _T("Unable to resolve constant buffer with identifier '%s' in surface shader '%s'.\n"), mBufferName.c_str(), pShader->getResourceName().c_str() );
            return false;
        
        } // End if failed

    } // End if shader linked
        
    // Allocate the system memory buffer area.
    mSystemBuffer = new cgByte[mDesc.length];

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : updateBuffer ()
/// <summary>
/// Populate the specified region of the buffer with new data. For optimal
/// performance it is recommended that the entire buffer is replaced where
/// possible. Specifying a sourceSize of 0 is equivalent to specifying a region 
/// starting at destinationOffset up to the end of the buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::updateBuffer( cgUInt32 destinationOffset, cgUInt32 sourceSize, void * sourceData )
{
    // Cannot update the resource if it is not loaded, or is already locked elsewhere
    if ( !mSystemBuffer || mLocked )
        return CG_NULL;

    // If sourceSize is 0, update entire buffer from the offset onwards.
    if ( !sourceSize )
        sourceSize = mDesc.length - destinationOffset;

    // Can we reliably discard the data in the existing buffer?
    cgUInt32 lockFlags = cgLockFlags::WriteOnly;
    if ( !destinationOffset && sourceSize == mDesc.length )
        lockFlags |= cgLockFlags::Discard;

    // Lock the vertex buffer
    void * lockedData = lock( destinationOffset, sourceSize, lockFlags );
    if ( !lockedData )
        return false;
    
    // Copy the data in.
    memcpy( lockedData, sourceData, sourceSize );

    // We're done.
    unlock();
    return true;
}

//-----------------------------------------------------------------------------
//  Name : lock ()
/// <summary>
/// Lock the specified area of the vertex buffer and return a pointer
/// to the underlying memory.
/// </summary>
//-----------------------------------------------------------------------------
void * cgConstantBuffer::lock( cgUInt32 OffsetToLock, cgUInt32 SizeToLock, cgUInt32 Flags )
{
    // Cannot lock the resource if it is not loaded, or is already locked elsewhere
    if ( !mSystemBuffer || mLocked )
        return CG_NULL;

    // Lock the buffer
    mLockedBuffer = &mSystemBuffer[OffsetToLock];
    mLocked = true;

    // Return the underlying buffer pointer
    return mLockedBuffer;
}

//-----------------------------------------------------------------------------
//  Name : unlock ()
/// <summary>
/// Unlock the buffer if previously locked.
/// </summary>
//-----------------------------------------------------------------------------
void cgConstantBuffer::unlock( )
{
    // Cannot unlock if it was not already locked
    if ( !mSystemBuffer || !mLocked )
        return;

    // Mark the data in the buffer as updated since the last call to the derived 
    // platform class' 'Apply()' method.
    mDataUpdated = true;

    // When unlocked, if applied to the device then the device's 
    // 'ConstantsDirty' member needs updating.
    if ( mBoundRegister >= 0 )
    {
        cgRenderDriver * pDriver = mManager->getRenderDriver();
        pDriver->mConstantsDirty |= (1 << mBoundRegister);
        
    } // End if currently bound
    
    // Item is no longer locked
    mLocked       = false;
    mLockedBuffer = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : getDesc ()
/// <summary>
/// Retrieve the layout descriptor for this constant buffer.
/// </summary>
//-----------------------------------------------------------------------------
const cgConstantBufferDesc & cgConstantBuffer::getDesc( ) const
{
    return mDesc;
}

//-----------------------------------------------------------------------------
//  Name : getConstantDesc ()
/// <summary>
/// Retrieve the descriptor for the specified constant.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::getConstantDesc( const cgString & strName, cgConstantDesc & Desc ) const
{
    cgUInt32 nOffset;
    return getConstantDesc( strName, Desc, nOffset );
}

//-----------------------------------------------------------------------------
//  Name : getConstantDesc ()
/// <summary>
/// Retrieve the descriptor for the specified constant.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::getConstantDesc( const cgString & strName, cgConstantDesc & Desc, cgUInt32 & nOffset ) const
{
    const cgConstantTypeDesc * pCurrentType = &mDesc;
    const cgConstantDesc * pConstant = CG_NULL;
    cgStringParser Parser;
    cgString strConstant;

    // Reset output variables.
    nOffset = 0;
    
    // Search through each of the components in the provided 
    // name string (i.e. MyConstant[3].MyConstant2 ).
    size_t nNameOffset = 0;
    while ( nNameOffset < strName.size() )
    {
        // Reset current constant
        pConstant = CG_NULL;

        // Find the first non-identifier character in the name.
        size_t nTokenEnd = strName.find_first_not_of( IdentifierChars, nNameOffset );

        // If none were found, we're at the end of the search
        // otherwise we must continue searching
        if ( nTokenEnd == cgString::npos )
        {
            strConstant = strName.substr( nNameOffset );
            nNameOffset = strName.size();

            // Find the constant associated with this name.
            for ( size_t i = 0; i < pCurrentType->constants.size(); ++i )
            {
                if ( pCurrentType->constants[i].name == strConstant )
                {
                    pConstant = &pCurrentType->constants[i];
                    break;

                } // End if matching name

            } // Next Constant

            // Return any matching constant descriptor
            if ( !pConstant )
                return false;
            nOffset += pConstant->offset;
            Desc = *pConstant;
            return true;

        } // End if none found

        // Find the constant associated with this token.
        strConstant = strName.substr( nNameOffset, nTokenEnd - nNameOffset );
        for ( size_t i = 0; i < pCurrentType->constants.size(); ++i )
        {
            if ( pCurrentType->constants[i].name == strConstant )
            {
                pConstant = &pCurrentType->constants[i];
                break;

            } // End if matching name

        } // Next Constant
        if ( !pConstant )
            return false;
        nOffset += pConstant->offset;

        // If the next non-identifier character opens array indexing,
        // resolve the element index. Remember, this may be multi-dimensional.
        size_t nFinalElement = 0, nDimension = 0;
        nNameOffset = nTokenEnd;
        while ( strName.at(nNameOffset) == _T('[') )
        {
            nNameOffset++;

            // Retrieve size component
            nTokenEnd = strName.find_first_not_of( NumericChars, nNameOffset );
            if ( nTokenEnd == cgString::npos || strName.at(nTokenEnd) != _T(']') )
            {
                cgToDoAssert( "Script Preprocessor", "Log missing ']'" );
                return false;
            
            } // End if malformed

            // Convert to integer representation
            size_t nElement;
            Parser.clear();
            Parser << strName.substr( nNameOffset, nTokenEnd - nNameOffset );
            Parser >> nElement;

            // Out of bounds for this dimension?
            if ( nDimension < pConstant->arrayDimensions.size() && 
                 nElement >= pConstant->arrayDimensions[nDimension] )
            {
                cgToDoAssert( "Script Preprocessor", "Array index out of bounds." );
                return false;

            } // End if out of bounds

            // Multiply by all subsequent array dimensions to establish
            // the correct starting location for the next dimension (if any).
            for ( size_t i = nDimension + 1; i < pConstant->arrayDimensions.size(); ++i )
                nElement *= pConstant->arrayDimensions[i];

            // Offset final element variable and move on.
            nFinalElement += nElement;
            nDimension++;

            // Offset to the next token in the name string
            nNameOffset = nTokenEnd + 1;

        } // Next array dimension

        // If the number of detected dimensions did not match
        // that of the original declaration, fail.
        if ( nDimension != pConstant->arrayDimensions.size() )
        {
            cgToDoAssert( "Script Preprocessor", "Incorrect number of array dimensions." )
            return false;
        
        } // End if malformed

        // Offset to correct element.
        nOffset += nFinalElement * pConstant->elementLength;

        // If there is more data to come, there should be a '.' separator.
        if ( nNameOffset >= strName.size() )
            break;
        if ( strName.at(nNameOffset) == _T('.') )
        {
            // Only applies if this is a UDT
            if ( !pConstant->isUDT )
            {
                cgToDoAssert( "Script Preprocessor", "Invalid subscript." )
                return false;
            
            } // End if !isUDT
            else
            {
                if ( pConstant->typeId >= (cgInt32)mTypes.size() )
                {
                    cgToDoAssert( "Script Preprocessor", "Mismatched or unknown typeid -- internal error." )
                    return false;
                
                } // End if invalid typeid
                pCurrentType = &mTypes[pConstant->typeId];

            } // End if isUDT

        } // End if '.'
        else
        {
            cgToDoAssert( "Script Preprocessor", "Syntax error. Expected '.'" )
            return false;
        
        } // End if !'.'
        nNameOffset++;
    
    } // Next token

    // Return valid constant descriptor
    if ( !pConstant )
        return false;
    Desc = *pConstant;
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setFloat ()
/// <summary>
/// Set the value of the specified constant to that of the provided single
/// precision float.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::setFloat( const cgString & strName, float Value )
{
    cgConstantDesc Desc;
    cgUInt32 nOffset;

    // Find the constant details.
    if ( !getConstantDesc( strName, Desc, nOffset ) )
        return false;
    
    // Lock the data area.
    float * pValue = (float*)lock( nOffset, 4, 0 );
    *pValue = Value;
    unlock();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVector ()
/// <summary>
/// Set the value of the specified constant to that of the provided two 
/// component vector.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::setVector( const cgString & strName, const cgVector2 & Value )
{
    cgConstantDesc Desc;
    cgUInt32 nOffset;

    // Find the constant details.
    if ( !getConstantDesc( strName, Desc, nOffset ) )
        return false;
    
    // Lock the data area.
    cgVector2 * pValue = (cgVector2*)lock( nOffset, 8, 0 );
    *pValue = Value;
    unlock();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVector ()
/// <summary>
/// Set the value of the specified constant to that of the provided three
/// component vector.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::setVector( const cgString & strName, const cgVector3 & Value )
{
    cgConstantDesc Desc;
    cgUInt32 nOffset;

    // Find the constant details.
    if ( !getConstantDesc( strName, Desc, nOffset ) )
        return false;
    
    // Lock the data area.
    cgVector3 * pValue = (cgVector3*)lock( nOffset, 12, 0 );
    *pValue = Value;
    unlock();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVector ()
/// <summary>
/// Set the value of the specified constant to that of the provided four
/// component vector.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::setVector( const cgString & strName, const cgVector4 & Value )
{
    cgConstantDesc Desc;
    cgUInt32 nOffset;

    // Find the constant details.
    if ( !getConstantDesc( strName, Desc, nOffset ) )
        return false;
    
    // Lock the data area.
    cgVector4 * pValue = (cgVector4*)lock( nOffset, 16, 0 );
    *pValue = Value;
    unlock();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setVector ()
/// <summary>
/// Set the value of the specified constant to that of the provided four
/// component vector (represetend by a color in this case).
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::setVector( const cgString & strName, const cgColorValue & Value )
{
    cgConstantDesc Desc;
    cgUInt32 nOffset;

    // Find the constant details.
    if ( !getConstantDesc( strName, Desc, nOffset ) )
        return false;

    // Lock the data area.
    cgColorValue * pValue = (cgColorValue*)lock( nOffset, 16, 0 );
    *pValue = Value;
    unlock();

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : setMatrix ()
/// <summary>
/// Set the value of the specified constant to that of the provided 4x4 matrix.
/// </summary>
//-----------------------------------------------------------------------------
bool cgConstantBuffer::setMatrix( const cgString & strName, const cgMatrix & Value )
{
    cgConstantDesc Desc;
    cgUInt32 nOffset;

    // Find the constant details.
    if ( !getConstantDesc( strName, Desc, nOffset ) )
        return false;
    
    // Lock the data area.
    cgMatrix * pValue = (cgMatrix*)lock( nOffset, sizeof(cgMatrix), 0 );
    *pValue = Value;

    // Unlock the buffer
    unlock();

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgConstantBufferLinker Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgConstantBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBufferLinker::cgConstantBufferLinker( cgConstantBufferDesc pBuffers[], size_t nBuffers, cgConstantTypeDesc pTypes[], size_t nTypes ) : 
    mBuffers(pBuffers), mBufferCount(nBuffers), mTypes(pTypes), mTypeCount(nTypes)
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgConstantBufferLinker () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBufferLinker::~cgConstantBufferLinker( )
{
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgConstantBufferLinker * cgConstantBufferLinker::createInstance( cgConstantBufferDesc pBuffers[], size_t nBuffers, cgConstantTypeDesc pTypes[], size_t nTypes )
{
    // Determine which driver we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    if ( Config.platform == cgPlatform::Windows )
    {
        switch ( Config.renderAPI )
        {
            case cgRenderAPI::Null:
                return CG_NULL;

#if defined( CGE_DX9_RENDER_SUPPORT )

            case cgRenderAPI::DirectX9:
                return new cgDX9ConstantBufferLinker( pBuffers, nBuffers, pTypes, nTypes );

#endif // CGE_DX9_RENDER_SUPPORT

#if defined( CGE_DX11_RENDER_SUPPORT )

            case cgRenderAPI::DirectX11:
                return new cgDX11ConstantBufferLinker( pBuffers, nBuffers, pTypes, nTypes );

#endif // CGE_DX11_RENDER_SUPPORT

        } // End switch renderAPI

    } // End if Windows
    return CG_NULL;
}