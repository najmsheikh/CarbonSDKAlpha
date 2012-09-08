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
// File : cgDX9ConstantBuffer.cpp                                            //
//                                                                           //
// Desc : Contains classes that represent blocks of shader constants that    //
//        can be applied to the device as a group. These constants most      //
//        often provide information to the shader library in order to        //
//        control their behavior (DX9 implementation).                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// API Support Check
//-----------------------------------------------------------------------------
#include <cgConfig.h>
#if defined( CGE_DX9_RENDER_SUPPORT )

//-----------------------------------------------------------------------------
// cgDX9ConstantBuffer Module Includes
//-----------------------------------------------------------------------------
#include <Resources/Platform/cgDX9ConstantBuffer.h>
#include <Resources/cgSurfaceShader.h>
#include <D3DX9.h>

///////////////////////////////////////////////////////////////////////////////
// cgDX9ConstantBuffer Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9ConstantBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9ConstantBuffer::cgDX9ConstantBuffer( cgUInt32 nReferenceId, const cgConstantBufferDesc & Desc ) : 
    cgConstantBuffer( nReferenceId, Desc )
{
    // Initialize variables to sensible defaults
    mLinker           = CG_NULL;
    mRegisterBuffer   = CG_NULL;
    mSharedLinker     = false;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9ConstantBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9ConstantBuffer::cgDX9ConstantBuffer( cgUInt32 nReferenceId, const cgConstantBufferDesc & Desc , const cgConstantTypeDesc::Array & aTypes ) : 
    cgConstantBuffer( nReferenceId, Desc, aTypes )
{
    // Initialize variables to sensible defaults
    mLinker           = CG_NULL;
    mRegisterBuffer   = CG_NULL;
    mSharedLinker     = false;
}

//-----------------------------------------------------------------------------
//  Name : cgDX9ConstantBuffer () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9ConstantBuffer::cgDX9ConstantBuffer( cgUInt32 nReferenceId, const cgSurfaceShaderHandle & hShader, const cgString & strBufferName ) : 
    cgConstantBuffer( nReferenceId, hShader, strBufferName )
{
    // Initialize variables to sensible defaults
    mLinker           = CG_NULL;
    mRegisterBuffer   = CG_NULL;
    mSharedLinker     = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9ConstantBuffer () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9ConstantBuffer::~cgDX9ConstantBuffer( )
{
    // Release resources
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgDX9ConstantBuffer::dispose( bool bDisposeBase )
{
    // We are in the process of disposing?
    mDisposing = true;

    // Release resources.
    unloadResource();

    // Dispose base(s).
    if ( bDisposeBase )
        cgConstantBuffer::dispose( true );
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
bool cgDX9ConstantBuffer::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_DX9ConstantBufferResource )
        return true;

    // Supported by base?
    return cgConstantBuffer::queryReferenceType( type );
}

//-----------------------------------------------------------------------------
//  Name : releaseConstantBuffer ()
/// <summary>
/// Release the internal constant buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9ConstantBuffer::releaseConstantBuffer( )
{
    // Call base class implementation.
    cgConstantBuffer::releaseConstantBuffer( );

    // Clean up 
    delete []mRegisterBuffer;
    mRegisterBuffer = CG_NULL;
    if ( !mSharedLinker )
        delete mLinker;
    mSharedLinker = false;
    mLinker = CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createConstantBuffer ()
/// <summary>
/// Create the internal constant buffer.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9ConstantBuffer::createConstantBuffer( )
{
    // Call base class implementation.
    if ( !cgConstantBuffer::createConstantBuffer( ) )
        return false;

    // Retrieve shared constant linker from specified shader?
    if ( mShader.isValid() )
    {
        cgSurfaceShader * pShader = mShader.getResource(true);
        if ( pShader->isLoaded() )
        {
            mLinker = pShader->getConstantBufferLinker();
            mSharedLinker = ( mLinker != CG_NULL );
            
        } // End if loaded
    
    } // End if shader linked

    // If no common / shared constant buffer linker was provided, construct a local one.
    if ( !mLinker )
    {
        mLinker = cgConstantBufferLinker::createInstance( &mDesc, 1, (mTypes.empty()) ? CG_NULL : &mTypes[0], mTypes.size() );
        cgToDo( "Effect Overhaul", "Remove following call when generateTransferMap() moves into the linker." )
        mLinker->generateConstantMappings();
        mSharedLinker = false;
    
    } // End if no linker

    // Generate the appropriate register transfer mappings for our data set.
    generateTransferMap( mDesc, 1, 0 );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : apply ()
/// <summary>
/// Apply the constant data in this buffer to the D3D device.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9ConstantBuffer::apply( IDirect3DDevice9 * pDevice )
{
    // If not already allocated, create a buffer that represents the final
    // layout of the data mapped to hardware constant registers that will 
    // allow us to set all of the data to the device in one go.
    if ( !mRegisterBuffer )
    {
        cgAssert( mDesc.registerCount != 0 );
        mRegisterBuffer = new cgByte[ mDesc.registerCount * sizeof(cgFloat) * 4 ];
        mDataUpdated = true;
    
    } // End if no buffer

    // Was the data modified recently?
    if ( mDataUpdated )
    {
        // Process the transfer map.
        for ( size_t i = 0; i < mTransferMap.size(); ++i )
        {
            const TransferItem & Item = mTransferMap[i];
            if ( !Item.transposeData && !Item.convertType )
            {
                // No special handling required. Straight block transfer.
                memcpy( mRegisterBuffer + Item.destinationOffset, mSystemBuffer + Item.sourceOffset, Item.size );

            } // End if no handling
            else if ( Item.convertType && !Item.transposeData )
            {
                // Item needs individually converting to 'float'.
                cgFloat * pBufferOut = (cgFloat*)(mRegisterBuffer+Item.destinationOffset);
                cgByte  * pBufferIn  = (mSystemBuffer + Item.sourceOffset);
                switch ( Item.baseType )
                {
                    case cgConstantType::Int:
                        cgAssert( Item.size == 4 );
                        *pBufferOut = (cgFloat)(*(cgInt32*)pBufferIn);
                        break;
                    case cgConstantType::UInt:
                        cgAssert( Item.size == 4 );
                        *pBufferOut = (cgFloat)(*(cgUInt32*)pBufferIn);
                        break;
                    case cgConstantType::Bool:
                        cgAssert( Item.size == 1 );
                        *pBufferOut = (*(bool*)pBufferIn) ? 1.0f : 0.0f;
                        break;
                    default:
                        cgAssertEx( false, "Unknown constant type." );
                        break;

                } // End switch type
            
            } // End if convert only
            else if ( Item.transposeData && !Item.convertType )
            {
                // Transpose the floating point data set
                cgUInt8 x, y;
                cgFloat * pBufferOut = (cgFloat*)(mRegisterBuffer+Item.destinationOffset);
                cgFloat * pBufferIn  = (cgFloat*)(mSystemBuffer + Item.sourceOffset);
                for ( y = 0; y < Item.rows; ++y )
                {
                    for ( x = 0; x < Item.columns; ++x )
                        *(pBufferOut + x + y * Item.columns) = *(pBufferIn + y + x * Item.rows);
                    
                } // Next row

            } // End if transpose only
            else
            {
                // Transpose AND convert the floating point data set
                cgUInt8 x, y;
                cgFloat * pBufferOut = (cgFloat*)(mRegisterBuffer+Item.destinationOffset);
                cgByte  * pBufferIn  = (mSystemBuffer + Item.sourceOffset);
                switch ( Item.baseType )
                {
                    case cgConstantType::Int:
                        cgAssert( Item.size == 4 );
                        for ( y = 0; y < Item.rows; ++y )
                        {
                            for ( x = 0; x < Item.columns; ++x )
                                *(pBufferOut + x + y * Item.columns) = (cgFloat)(*(((cgInt32*)pBufferIn) + y + x * Item.rows));
                            
                        } // Next row
                        break;
                    case cgConstantType::UInt:
                        cgAssert( Item.size == 4 );
                        for ( y = 0; y < Item.rows; ++y )
                        {
                            for ( x = 0; x < Item.columns; ++x )
                                *(pBufferOut + x + y * Item.columns) = (cgFloat)(*(((cgUInt32*)pBufferIn) + y + x * Item.rows));
                            
                        } // Next row
                        break;
                    case cgConstantType::Bool:
                        cgAssert( Item.size == 1 );
                        for ( y = 0; y < Item.rows; ++y )
                        {
                            for ( x = 0; x < Item.columns; ++x )
                                *(pBufferOut + x + y * Item.columns) = (*(((bool*)pBufferIn) + y + x * Item.rows)) ? 1.0f : 0.0f;
                            
                        } // Next row
                        break;
                    default:
                        cgAssertEx( false, "Unknown constant type." );
                        break;

                } // End switch type
                

            } // End if transpose and convert

        } // Next instruction

        // Reset data updated flag
        mDataUpdated = false;

    } // End if buffer dirty.

    // Apply to the device
    if ( mDesc.bindVS )
        pDevice->SetVertexShaderConstantF( mDesc.globalOffset, (float*)mRegisterBuffer, mDesc.registerCount );
    if ( mDesc.bindPS )
        pDevice->SetPixelShaderConstantF( mDesc.globalOffset, (float*)mRegisterBuffer, mDesc.registerCount );

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : generateTransferMap () (Protected)
/// <summary>
/// Generate a list of copy operations that describe how data should be
/// transferred from the system memory (CPU aligned) buffer into the register
/// mapped (GPU aligned) buffer.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9ConstantBuffer::generateTransferMap( const cgConstantTypeDesc & ctDesc, cgUInt32 nNumElements, cgUInt32 nDestRegisterOffset, bool bRootType /* = true */ )
{
    cgToDo( "Effect Overhaul", "Move into the linker and cache so that we can take advantage of the shared nature of the linker members." )
    cgToDo( "Effect Overhaul", "When moved into the linker, make sure that it calls 'generateConstantMappings()' first if mappings haven't already been generated( like generateBufferDeclarations() )" )

    for ( cgUInt32 nElement = 0; nElement < nNumElements; ++nElement )
    {
        for ( size_t i = 0; i < ctDesc.constants.size(); ++i )
        {
            const cgConstantDesc & cDesc = ctDesc.constants[i];
            if ( cDesc.isUDT )
            {
                // UDTs necessitate recursion.
                generateTransferMap( mTypes[cDesc.typeId], cDesc.elements, nDestRegisterOffset + cDesc.registerOffset, false );
            
            } // End if UDT
            else
            {
                // First work out if we can block transfer the entire member into the 
                // register buffer. This is only possible if the item uses a float as
                // its base primitive type, has only a single row when not an array, or 
                // is single row with four fully populated columns when it is an array.
                bool bBlockTransfer = false;
                if ( cDesc.typeId == cgConstantType::Float )
                {
                    if ( !cDesc.isArray && cDesc.rows == 1 )
                        bBlockTransfer = true;
                    else if ( cDesc.isArray && cDesc.columns == 4 && cDesc.rows == 1 )
                        bBlockTransfer = true;
                
                } // End if float

                // Compute the offset of this constant (in bytes) as it will exist in
                // the source memory buffer.
                size_t nSrcOffset = (nElement * ctDesc.length) + cDesc.offset;

                // Compute the offset of this constant (in bytes) as it will exist in
                // the destination register buffer.
                size_t nDestOffset;
                if ( cDesc.registerPacking )
                    nDestOffset = (((nDestRegisterOffset + cDesc.registerOffset) * 4) + cDesc.registerComponent) * sizeof(float);
                else
                    nDestOffset = ((nDestRegisterOffset + cDesc.registerOffset) * 4) * sizeof(float);

                // If we were able to block transfer, just insert a new transfer record.
                // Otherwise we need take a more complex path which potentially splits up,
                // transposes and / or converts data types on the fly.
                if ( bBlockTransfer )
                {
                    // This is the "simple" case where we can just copy the entire
                    // member over into the destination register buffer as-is. First, 
                    // work out if we can merge the transfer of this constant data
                    // into the prior record? 
                    bool bCanMerge = false;
                    if ( !mTransferMap.empty() )
                    {
                        TransferItem & PrevItem = mTransferMap.back();
                        if ( !PrevItem.transposeData && !PrevItem.convertType &&
                             nSrcOffset == PrevItem.sourceOffset + PrevItem.size && 
                             nDestOffset == PrevItem.destinationOffset + PrevItem.size )
                            bCanMerge = true;

                    } // End if !first

                    // Merge or create new record?
                    if ( bCanMerge )
                    {
                        // Merge with prior record.
                        TransferItem & PrevItem = mTransferMap.back();
                        if ( cDesc.registerPacking )
                            PrevItem.size += cDesc.columns * sizeof(float);
                        else
                            PrevItem.size += cDesc.elementLength * cDesc.elements;
                    
                    } // End if merge
                    else
                    {
                        // New record.
                        mTransferMap.resize( mTransferMap.size() + 1 );
                        TransferItem & NewItem = mTransferMap.back();
                        
                        // Compute basic transfer details (offset, size, etc.)
                        NewItem.sourceOffset = nSrcOffset;
                        NewItem.destinationOffset = nDestOffset;
                        if ( cDesc.registerPacking )
                            NewItem.size = cDesc.columns * sizeof(float);
                        else
                            NewItem.size = cDesc.elementLength * cDesc.elements;
                            //NewItem.size = (cDesc.registerCount * sizeof(float) * 4) * cDesc.elements;

                        // Describe any special handling.
                        NewItem.transposeData  = false;
                        NewItem.rows           = 0;
                        NewItem.columns        = 0;
                        NewItem.convertType    = false;
                        NewItem.baseType       = cDesc.typeId;

                    } // End if !merge
                    
                } // End if bBlockTransfer
                else
                {
                    // Add a transfer item for each element (to support arrays). In DX9, 
                    // each individual element starts on a new register irrespective of the 
                    // number of columns in the data type.
                    for ( size_t j = 0; j < cDesc.elements; ++j )
                    {
                        mTransferMap.resize( mTransferMap.size() + 1 );
                        TransferItem & NewItem      = mTransferMap.back();

                        // Compute basic transfer details (offset, size, etc.)
                        NewItem.sourceOffset          = nSrcOffset + (j * cDesc.elementLength);
                        NewItem.destinationOffset          = nDestOffset + (j * cDesc.rows * sizeof(float) * 4);
                        NewItem.size               = cDesc.elementLength;

                        // Describe any special handling. Matrices require transposing.
                        // Floating point types can be transferred largely as is,
                        // otherwise each individual element needs to be converted.
                        NewItem.transposeData      = ( cDesc.rows > 1 && cDesc.columns > 1 );
                        NewItem.rows               = (cgUInt8)cDesc.rows;
                        NewItem.columns            = (cgUInt8)cDesc.columns;
                        NewItem.convertType        = ( cDesc.typeId != cgConstantType::Float );
                        NewItem.baseType           = cDesc.typeId;

                    } // Next element
                    
                } // End if !bBlockTransfer

            } // End if !UDT

        } // Next Constant

        // Move onto next set of register for the next element
        nDestRegisterOffset += ctDesc.registerCount;

    } // Next array element
}

//-----------------------------------------------------------------------------
//  Name : loadResource ()
/// <summary>
/// If deferred loading is employed, load the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9ConstantBuffer::loadResource( )
{
    // Already loaded?
    if ( mResourceLoaded )
        return true;

    // Attempt to create the constant buffer
    if ( !createConstantBuffer() )
        return false;

    // We're now loaded
    mResourceLoaded = true;

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : unloadResource ()
/// <summary>
/// If deferred loading is employed, destroy the underlying resources.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9ConstantBuffer::unloadResource( )
{
    // Release the constant buffer
    releaseConstantBuffer();

    // We are no longer loaded
    mResourceLoaded = false;

    // Success!
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// cgDX9ConstantBufferLinker Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgDX9ConstantBufferLinker () (Constructor)
/// <summary>
/// Constructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9ConstantBufferLinker::cgDX9ConstantBufferLinker( cgConstantBufferDesc pBuffers[], size_t nBuffers, cgConstantTypeDesc pTypes[], size_t nTypes ) : 
    cgConstantBufferLinker( pBuffers, nBuffers, pTypes, nTypes )
{
    mMappingsGenerated = false;
}

//-----------------------------------------------------------------------------
//  Name : ~cgDX9ConstantBufferLinker () (Destructor)
/// <summary>
/// Destructor for this class.
/// </summary>
//-----------------------------------------------------------------------------
cgDX9ConstantBufferLinker::~cgDX9ConstantBufferLinker( )
{
}

//-----------------------------------------------------------------------------
//  Name : generateConstantMappings () (Virtual)
/// <summary>
/// Generate mappings between the constants as they exist within the system
/// memory constant buffer, and the hardware registers into which the constants
/// will be mapped. Modifies the original buffer, type and constant descriptors
/// to ensure that this information is globally accessible.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9ConstantBufferLinker::generateConstantMappings( )
{
    // Clear prior details.
    mReferencedTypeHandles.clear();

    // First, resolve all user defined types referenced by the cbuffers.
    cgInt32Set VisitedTypeHandles;
    for ( size_t i = 0; i < mBufferCount; ++i )
    {
        cgConstantBufferDesc & cbDesc = mBuffers[i];
        for ( size_t j = 0; j < cbDesc.constants.size(); ++j )
        {
            // If this constant makes use of a user defined type then
            // we must add it to the referenced type array (above) in
            // addition to processing any /additional/ UDTs that those 
            // child types reference in a similar fashion (recursively).
            cgConstantDesc & cDesc = cbDesc.constants[j];
            if ( cDesc.isUDT )
                collectReferencedUDTs( VisitedTypeHandles, mReferencedTypeHandles, cDesc.typeId );

            // While we're here, make sure to reset the 'registerOffset'
            // value to a value we can identify as 'not yet computed'.
            cDesc.registerOffset    = 0xFFFFFFFF;
            cDesc.registerCount     = 0;
            cDesc.registerComponent = 0;
            cDesc.registerPacking   = false;

        } // Next constant

    } // Next buffer

    // Now that we have a list of all referenced types -- ordered bottom most
    // child type to top -- process the constants in those types and generate
    // register mapping details. Since constants in structures cannot be packed
    // in DX9 (default packing rules must apply), this is the simpler part of the
    // process which simply involves counting registers.
    for ( size_t i = 0; i < mReferencedTypeHandles.size(); ++i )
    {
        cgConstantTypeDesc & ctDesc = mTypes[mReferencedTypeHandles[i]];
        ctDesc.registerCount = 0;
        
        // Process each of the constants and work out register offsets / sizes.
        for ( size_t j = 0; j < ctDesc.constants.size(); ++j )
        {
            cgConstantDesc & cDesc = ctDesc.constants[j];
            
            // Each member in a structure starts a new register in DX9
            cDesc.registerOffset    = ctDesc.registerCount;
            cDesc.registerComponent = 0;
            cDesc.registerCount     = 0;
            cDesc.registerPacking   = false;

            // References user defined type?
            if ( cDesc.isUDT )
            {
                // This is a UDT member. The number of registers it consumes is simply
                // equal to the number of registers consumed by the referenced type
                // multiplied by the number of array elements.
                cDesc.registerCount = mTypes[cDesc.typeId].registerCount * cDesc.elements;
            
            } // End if UDT
            else
            {
                // This is a standard 'primitive' member. The Number of registers is 
                // equivalent to the number of rows times the number of elements. In 
                // DX9, the column count is irrelevant (i.e. even a single float MyVar[4] 
                // will consume four constants)
                cDesc.registerCount = cDesc.rows * cDesc.elements;

            } // End if !UDT

            // Increase size of parent type based on size of this constant.
            ctDesc.registerCount += cDesc.registerCount;

        } // Next Constant

    } // Next Type

    // With register mappings complete for the user defined types, now we must
    // do the same for the constant buffers themselves. Since it's possible to
    // perform rudimentary packing of the cbuffer members in DX9 (with some 
    // #define tricks) this process is slightly more involved. In this case we
    // must search for members with similar types (i.e. two float2 members, or
    // perhaps one int3 and a single int) and attempt to pack them together into
    // a single shader constant register.
    for ( size_t i = 0; i < mBufferCount; ++i )
    {
        cgConstantBufferDesc & cbDesc = mBuffers[i];
        
        // Keep track of the current offset of the hardware constant 
        // we're outputting to (values expressed relative to the buffer's
        // 'globalOffset' value).
        cbDesc.registerCount = 0;
        cgUInt8 nCurrentComponent  = 0;

        // Process the buffer constants
        for ( size_t j = 0; j < cbDesc.constants.size(); ++j )
        {
            // Skip if we've already output this buffer constant.
            cgConstantDesc & cCurr = cbDesc.constants[j];
            if ( cCurr.registerOffset != 0xFFFFFFFF )
                continue;

            // Process the next unused constant. Constants that are compatible with
            // the packing procedure must only have one row, less than 4 columns and 
            // must not be an array or use a user defined type.
            if ( cCurr.rows == 1 && cCurr.columns < 4 && !cCurr.isArray && !cCurr.isUDT )
            {
                // Search for other constants that fit the same critera and share the 
                // same base type (i.e. both floating point in nature). If there is 
                // enough room left in the current hardware constant, it can be packed. 
                // We start at the /current/ buffer constant so that it will be automatically
                // assigned using the same code path as everything else.
                for ( size_t k = j; k < cbDesc.constants.size(); ++k )
                {
                    // Skip constants that have already been included.
                    cgConstantDesc & cTest = cbDesc.constants[k];
                    if ( cTest.registerOffset != 0xFFFFFFFF )
                        continue;
                    
                    // Constant is first-stage compatible (1 row, is not a UDT or array and has 
                    // a matching primitive type to that of the current constant)?
                    if ( k == j || (cTest.rows == 1 && !cTest.isArray && !cTest.isUDT && cTest.typeId == cCurr.typeId) )
                    {
                        // Enough space remaining in current hardware constant for this value?
                        if ( (signed)cTest.columns <= (4 - nCurrentComponent) )
                        {
                            // Value fits in remaining register components and is therefore
                            // compatible for packing. Assign to the current register.
                            cTest.registerOffset    = cbDesc.registerCount;
                            cTest.registerComponent = nCurrentComponent;
                            cTest.registerCount     = 1;
                            cTest.registerPacking   = true;
                
                            // More register components have now been populated equivalent to
                            // the number of columns in the constant.
                            nCurrentComponent += (cgUInt8)cTest.columns;

                            // End the search if the register is now full.
                            if ( nCurrentComponent == 4 )
                                break;

                        } // End if fits

                    } // End if compatible

                } // Next Constant

                // Register has been filled. Move on to the next.
                cbDesc.registerCount++;
                nCurrentComponent = 0;

            } // End if can pack
            else
            {
                // Cannot pack. Just output as is.
                cCurr.registerOffset = cbDesc.registerCount;

                // Compute the register count for this member. For a UDT, this is simply equivalent to 
                // the number of registers consumed by the reference type multiplied by the number of
                // elements (if it's an array). For primitive types, this is equal to the number of rows 
                // multiplied by the number of elements (if it's an array).
                if ( cCurr.isUDT )
                    cCurr.registerCount = mTypes[cCurr.typeId].registerCount * cCurr.elements;
                else
                    cCurr.registerCount = cCurr.rows * cCurr.elements;

                // Grow buffer size appropriately.
                cbDesc.registerCount += cCurr.registerCount;
                nCurrentComponent = 0;
                    
            } // End if no packing

        } // Next Constant

    } // Next Buffer

    // Mappings have now been generated.
    mMappingsGenerated = true;
}

//-----------------------------------------------------------------------------
//  Name : generateBufferDeclarations () (Virtual)
/// <summary>
/// Generate compatible shader code declarations for the referenced 
/// constant buffer & type declarations.
/// </summary>
//-----------------------------------------------------------------------------
bool cgDX9ConstantBufferLinker::generateBufferDeclarations( cgInt32 pBufferRefs[], size_t nBuffers, cgString & strOut )
{
    static const cgTChar * PackingFormats[4][4] =
    {
        {
            _T("#define %s (_phwc%i.x)\n"),
            _T("#define %s (_phwc%i.y)\n"),
            _T("#define %s (_phwc%i.z)\n"),
            _T("#define %s (_phwc%i.w)\n")
        },
        {
            _T("#define %s (_phwc%i.xy)\n"),
            _T("#define %s (_phwc%i.yz)\n"),
            _T("#define %s (_phwc%i.zw)\n"),
            CG_NULL
        },
        {
            _T("#define %s (_phwc%i.xyz)\n"),
            _T("#define %s (_phwc%i.yzw)\n"),
            CG_NULL,
            CG_NULL
        },
        {
            _T("#define %s (_phwc%i.xyzw)\n"),
            CG_NULL,
            CG_NULL,
            CG_NULL
        }
    };

    // If constants mappings have not already been generated, process them now.
    if ( !mMappingsGenerated )
        generateConstantMappings();

    // Now generate the shader code (or retrieve from the cache) for each of the
    // above user defined types referenced by the requested cbuffers.
    for ( size_t i = 0; i < mReferencedTypeHandles.size(); ++i )
    {
        // Has code already been generated for this type on a prior occassion?
        HandleCodeMap::const_iterator itCode = mCachedTypeCode.find( mReferencedTypeHandles[i] );
        if ( itCode != mCachedTypeCode.end() )
        {
            // Cached version was discovered. Just use it.
            strOut.append( itCode->second );
            strOut.append( _T("\n") );
        
        } // End if cached
        else
        {
            // Ask the linker to generate code for this type.
            cgString strCode;
            generateTypeDeclaration( mTypes[mReferencedTypeHandles[i]], strCode );

            // Inserted into the cache for later retrieval
            mCachedTypeCode[ mReferencedTypeHandles[i] ] = strCode;

            // Attach to the type string
            strOut.append( strCode );
            strOut.append( _T("\n") );
            
        } // End if !cached

    } // Next Type

    // Now that the types are output, it's time to output the constants themselves.
    // We use a packing mechanism that (as much as possible) packs together like
    // typed constants (float2 with float2 for instance) in order to allow as few 
    // constants to be uploaded as possible. These mappings should already have
    // been generated with an earlier call to 'generateConstantMappings()'.
    for ( size_t i = 0; i < nBuffers; ++i )
    {
        const cgConstantBufferDesc & cbDesc = mBuffers[pBufferRefs[i]];

        // Output some debug information            
        #if defined(_DEBUG)
            strOut.append( cgString::format( _T("\n////////////////////////////////\n") ) );
            strOut.append( cgString::format( _T("// cbuffer %s\n"), cbDesc.name.c_str() ) );
            strOut.append( cgString::format( _T("////////////////////////////////\n") ) );
            strOut.append( cgString::format( _T("// Unpacked constants\n") ) );
        #endif

        // Output the unpacked constant declarations first and also determine
        // which 'packing' registers will be required (including their type).
        std::map<cgInt32, cgConstantType::Base> PackingRegisters;
        for ( size_t j = 0; j < cbDesc.constants.size(); ++j )
        {
            const cgConstantDesc & cCurr = cbDesc.constants[j];
            if ( !cCurr.registerPacking )
            {
                // Unpacked constant. Output as-is.
                // First, output primitive type (i.e. if this was "float3x3", 
                // primitive type is "float")
                strOut.append( cCurr.primitiveTypeName );
                
                // Explicit matrix? (i.e. float2x2, int3x2, etc.)
                // Shaders are compiled as column major, so row/column size must be 
                // transposed relative to the original row major type descriptor!
                if ( cCurr.columns > 1 && cCurr.rows > 1 )
                    strOut.append( cgString::format( _T("%ix%i"), cCurr.columns, cCurr.rows ) );
                else if ( cCurr.rows > 1 )
                    strOut.append( cgString::format( _T("1x%i"), cCurr.rows ) );
                else if ( cCurr.columns > 1 )
                    strOut.append( cgString::format( _T("%i"), cCurr.columns ) );
                
                // Output identifier and register binding.
                strOut.append( _T(" ") );
                strOut.append( cCurr.name );
                if ( cCurr.isArray )
                {
                    for ( size_t k = 0; k < cCurr.arrayDimensions.size(); ++k )
                        strOut.append( cgString::format( _T("[%i]"), cCurr.arrayDimensions[k] ) );
                
                } // End if isArray
                strOut.append( cgString::format( _T(" : register(c%i);\n"), cCurr.registerOffset + cbDesc.globalOffset ) );

            } // End if no packing
            else
            {
                PackingRegisters[cCurr.registerOffset + cbDesc.globalOffset] = (cgConstantType::Base)cCurr.typeId;
            
            } // End if packed

        } // Next Constant

        // Output some debug information            
        #if defined(_DEBUG)
            strOut.append( cgString::format( _T("\n// Packing constant registers\n") ) );
        #endif

        // Now output packing register details.
        std::map<cgInt32, cgConstantType::Base>::const_iterator itRegister;
        for ( itRegister = PackingRegisters.begin(); itRegister != PackingRegisters.end(); ++itRegister )
        {
            // Insert constant of the appropriate type
            switch ( itRegister->second )
            {
                case cgConstantType::Float:
                    strOut.append( cgString::format( _T("float4 _phwc%i : register(c%i);\n"), itRegister->first, itRegister->first ) );
                    break;
                case cgConstantType::Int:
                    strOut.append( cgString::format( _T("int4 _phwc%i : register(c%i);\n"), itRegister->first, itRegister->first ) );
                    break;
                case cgConstantType::UInt:
                    strOut.append( cgString::format( _T("uint4 _phwc%i : register(c%i);\n"), itRegister->first, itRegister->first ) );
                    break;
                case cgConstantType::Bool:
                    strOut.append( cgString::format( _T("bool4 _phwc%i : register(c%i);\n"), itRegister->first, itRegister->first ) );
                    break;
                default:
                    cgAssertEx( false, "Unknown constant type." );
                    break;
            
            } // End switch typeId

        } // Next register

        // Output some debug information            
        #if defined(_DEBUG)
            strOut.append( cgString::format( _T("\n// Packed constant definitions\n") ) );
        #endif
        
        // Now output packed constant #defines
        for ( size_t j = 0; j < cbDesc.constants.size(); ++j )
        {
            const cgConstantDesc & cCurr = cbDesc.constants[j];
            if ( cCurr.registerPacking )
            {
                const cgTChar * lpszFormat = PackingFormats[cCurr.columns-1][cCurr.registerComponent];
                strOut.append( cgString::format( lpszFormat, cCurr.name.c_str(), cCurr.registerOffset + cbDesc.globalOffset ) );
             
            } // End if packed

        } // Next Constant

        // Output some debug information            
        #if defined(_DEBUG)
            strOut.append( cgString::format( _T("\n// Registers consumed = %i\n"), cbDesc.registerCount ) );
            strOut.append( cgString::format( _T("////////////////////////////////\n") ) );
        #endif

    } // Next Buffer
        
    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : collectReferencedUDTs () (Protected, Recursive)
/// <summary>
/// Recursively collect a list of all user defined constant types that are
/// referenced by the specified type and any child types.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9ConstantBufferLinker::collectReferencedUDTs( cgInt32Set & VisitedTypeHandles, cgInt32Array & aReferencedTypeHandles, cgInt32 nTypeId )
{
    // Skip if this type has already been visited.
    if ( VisitedTypeHandles.find(nTypeId) != VisitedTypeHandles.end() )
        return;

    // Now mark this type as visited as the first thing we do (before recursing).
    // This prevents duplicate references from being inserted if this (or a child) 
    // structure self-references.
    VisitedTypeHandles.insert( nTypeId );

    // Before we add this to the referenced type handle array we must recurse.
    // In effect, we must add the type handles to the array in a 'bottom-up'
    // fashion to ensure correct ordering of type declarations in the output code.
    const cgConstantTypeDesc & ctDesc = mTypes[nTypeId];
    for ( size_t i = 0; i < ctDesc.constants.size(); ++i )
    {
        // If this constant makes use of a user defined type, recurse.
        const cgConstantDesc & cDesc = ctDesc.constants[i];
        if ( cDesc.isUDT )
            collectReferencedUDTs( VisitedTypeHandles, aReferencedTypeHandles, cDesc.typeId );

    } // Next constant

    // Add this type to the (correctly ordered) type handle array.
    aReferencedTypeHandles.push_back( nTypeId );
}

//-----------------------------------------------------------------------------
//  Name : generateTypeDeclaration () (Protected)
/// <summary>
/// Generate compatible shader code declarations for the specified user
/// defined constant type / structure.
/// </summary>
//-----------------------------------------------------------------------------
void cgDX9ConstantBufferLinker::generateTypeDeclaration( const cgConstantTypeDesc & Desc, cgString & strOut )
{
    // Output structure header
    strOut.append( _T("struct ") );
    strOut.append( Desc.name );
    strOut.append( _T("{\n") );

    // Output structure members.
    for ( size_t i = 0; i < Desc.constants.size(); ++i )
    {
        const cgConstantDesc & cDesc = Desc.constants[i];

        // Output primitive type (i.e. if this was "float3x3", primitive type is "float")
        strOut.append( cDesc.primitiveTypeName );
        
        // Explicit matrix? (i.e. float2x2, int3x2, etc.)
        // Shaders are compiled as column major, so row/column size must be 
        // transposed relative to the original row major type descriptor!
        if ( cDesc.columns > 1 && cDesc.rows > 1 )
            strOut.append( cgString::format( _T("%ix%i"), cDesc.columns, cDesc.rows ) );
        else if ( cDesc.rows > 1 )
            strOut.append( cgString::format( _T("1x%i"), cDesc.rows ) );
        else if ( cDesc.columns > 1 )
            strOut.append( cgString::format( _T("%i"), cDesc.columns ) );
        
        // Output identifier
        strOut.append( _T(" ") );
        strOut.append( cDesc.name );
        if ( cDesc.isArray )
        {
            for ( size_t j = 0; j < cDesc.arrayDimensions.size(); ++j )
                strOut.append( cgString::format( _T("[%i]"), cDesc.arrayDimensions[j] ) );
        
        } // End if isArray
        strOut.append( _T(";\n") );

    } // Next Member

    // Close structure
    strOut.append( _T("};\n") );
}

#endif // CGE_DX9_RENDER_SUPPORT