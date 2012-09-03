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
// Name : cgVertexFormats.cpp                                                //
//                                                                           //
// Desc : Contains the various vertex format classes used to construct the   //
//        different types of geometry within the scene.                      //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgVertexFormats Module Includes
//-----------------------------------------------------------------------------
#include <Rendering/cgVertexFormats.h>

//-----------------------------------------------------------------------------
// Static Variable Definitions
//-----------------------------------------------------------------------------
cgVertexFormat                 * cgVertexFormat::mDefaultFormat = CG_NULL;
cgVertexFormat::FormatDatabase   cgVertexFormat::mFormats;

//-----------------------------------------------------------------------------
// Module Local Variables
//-----------------------------------------------------------------------------
namespace
{
    static const int MaxDeclElementIndex = 19;   // The maximum 'index' for a given vertex element in a declarator

} // End Unnamed Namespace

///////////////////////////////////////////////////////////////////////////////
// cgVertexFormat Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgVertexFormat () (Constructor)
/// <summary>
/// cgVertexFormat Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat::cgVertexFormat( )
{
    // Initialize variables to sensible defaults
    mElements         = CG_NULL;
    mUsageMap         = CG_NULL;
    mElementCount     = 0;
    mStride           = 0;
    mInDatabase       = false;
    mElementsDirty    = false;
}

//-----------------------------------------------------------------------------
//  Name : cgVertexFormat () (Copy Constructor)
/// <summary>
/// cgVertexFormat alternate class constructor. Used to make a duplicate
/// of the specified format at construction time.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat::cgVertexFormat( const cgVertexFormat & SrcFormat )
{
    // Initialize variables to sensible defaults
    mElements         = CG_NULL;
    mUsageMap         = CG_NULL;
    mElementCount     = 0;
    mStride           = 0;
    mInDatabase       = false;
    mElementsDirty    = false;

    // Copy the format using the assignment operator
    *this = SrcFormat;
}

//-----------------------------------------------------------------------------
//  Name : ~cgVertexFormat () (Destructor)
/// <summary>
/// cgVertexFormat Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat::~cgVertexFormat( )
{
    // Release allocated memory
    delete []mElements;
    delete []mUsageMap;

    // Clear variables
    mElements     = CG_NULL;
    mUsageMap     = CG_NULL;
    mElementCount = 0;
    mStride       = 0;
}

//-----------------------------------------------------------------------------
//  Name : getElementOffset ()
/// <summary>
/// Retrieve the offset of the element in a data array of this format.
/// </summary>
//-----------------------------------------------------------------------------
int cgVertexFormat::getElementOffset( cgUInt32 Usage, cgUInt8 Index /* = 0 */ )
{
    // Retrieve the specified vertex element
    const D3DVERTEXELEMENT9 * pElement = getElement( Usage, Index );
    if ( !pElement ) return -1;

    // Return the offset for this element in bytes
    return pElement->Offset;
}

//-----------------------------------------------------------------------------
//  Name : getElement ()
/// <summary>
/// Retrieve the element from the declarator based on it's usage
/// </summary>
//-----------------------------------------------------------------------------
const D3DVERTEXELEMENT9 * cgVertexFormat::getElement( cgUInt32 Usage, cgUInt8 Index /* = 0 */ )
{
    // Rebuild usage data if necessary.
    if ( mElementsDirty )
        buildUsageData();

    // Bail if out of range
    if ( !mUsageMap || Usage > 13 || Index > MaxDeclElementIndex )
        return CG_NULL;

    // Retrieve the correct element
    return mUsageMap[ Usage + (Index * (MaxDeclElementIndex + 1)) ];
}

//-----------------------------------------------------------------------------
//  Name : getVertexPosition ()
/// <summary>
/// Retrieve the position of the vertex if this information exists.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 * cgVertexFormat::getVertexPosition( cgByte * pData )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_POSITION );
    if ( nOffset < 0 ) return CG_NULL;
    return (cgVector3*)(pData + nOffset);
}

//-----------------------------------------------------------------------------
//  Name : getVertexNormal ()
/// <summary>
/// Retrieve the normal of the vertex if this information exists.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 * cgVertexFormat::getVertexNormal( cgByte * pData )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_NORMAL );
    if ( nOffset < 0 ) return CG_NULL;
    return (cgVector3*)(pData + nOffset);
}

//-----------------------------------------------------------------------------
//  Name : getVertexBinormal ()
/// <summary>
/// Retrieve the binormal of the vertex if this information exists.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 * cgVertexFormat::getVertexBinormal( cgByte * pData )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_BINORMAL );
    if ( nOffset < 0 ) return CG_NULL;
    return (cgVector3*)(pData + nOffset);
}

//-----------------------------------------------------------------------------
//  Name : getVertexTangent ()
/// <summary>
/// Retrieve the tangent of the vertex if this information exists.
/// </summary>
//-----------------------------------------------------------------------------
cgVector3 * cgVertexFormat::getVertexTangent( cgByte * pData )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_TANGENT );
    if ( nOffset < 0 ) return CG_NULL;
    return (cgVector3*)(pData + nOffset);
}

//-----------------------------------------------------------------------------
//  Name : getVertexTextureCoords ()
/// <summary>
/// Retrieve the specified texture coordinate of the vertex if this 
/// information exists.
/// </summary>
//-----------------------------------------------------------------------------
cgFloat * cgVertexFormat::getVertexTextureCoords( cgByte * pData, cgUInt8 Index, cgUInt8 Component /* = 0 */ )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_TEXCOORD, Index );
    if ( nOffset < 0 ) return CG_NULL;
    return (cgFloat*)(pData + nOffset + (Component * sizeof(cgFloat)) );
}

//-----------------------------------------------------------------------------
//  Name : setVertexPosition ()
/// <summary>
/// Set the position of the vertex if this information is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setVertexPosition( cgByte * pData, const cgVector3 & value  )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_POSITION );
    if ( nOffset < 0 ) return;
    *(cgVector3*)(pData + nOffset) = value;
}

//-----------------------------------------------------------------------------
//  Name : setVertexNormal ()
/// <summary>
/// Set the normal of the vertex if this information is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setVertexNormal( cgByte * pData, const cgVector3 & value )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_NORMAL );
    if ( nOffset < 0 ) return;
    *(cgVector3*)(pData + nOffset) = value;
}

//-----------------------------------------------------------------------------
//  Name : setVertexBinormal ()
/// <summary>
/// Set the binormal of the vertex if this information is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setVertexBinormal( cgByte * pData, const cgVector3 & value )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_BINORMAL );
    if ( nOffset < 0 ) return;
    *(cgVector3*)(pData + nOffset) = value;
}

//-----------------------------------------------------------------------------
//  Name : setVertexTangent ()
/// <summary>
/// Set the tangent of the vertex if this information is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setVertexTangent( cgByte * pData, const cgVector3 & value )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_TANGENT );
    if ( nOffset < 0 ) return;
    *(cgVector3*)(pData + nOffset) = value;
}

//-----------------------------------------------------------------------------
//  Name : setVertexTextureCoords ()
/// <summary>
/// Set the specified texture coordinate of the vertex if this 
/// information is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setVertexTextureCoords( cgByte * pData, cgUInt8 Index, cgUInt8 Component, cgFloat value )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_TEXCOORD, Index );
    if ( nOffset < 0 ) return;
    *(cgFloat*)(pData + nOffset + (Component * sizeof(cgFloat)) ) = value;
}

//-----------------------------------------------------------------------------
//  Name : setVertexBlendIndices ()
/// <summary>
/// Set the blend indices of the vertex if this information is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setVertexBlendIndices( cgByte * pData, cgUInt32 value )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_BLENDINDICES );
    if ( nOffset < 0 ) return;
    *(cgUInt32*)(pData + nOffset) = value;
}

//-----------------------------------------------------------------------------
//  Name : setVertexBlendWeights ()
/// <summary>
/// Set the blend weights of the vertex if this information is needed.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setVertexBlendWeights( cgByte * pData, cgVector4 & value )
{
    int nOffset = getElementOffset( D3DDECLUSAGE_BLENDWEIGHT );
    if ( nOffset < 0 ) return;
    *(cgVector4*)(pData + nOffset) = value;
}

//-----------------------------------------------------------------------------
//  Name : getStride ()
/// <summary>
/// Retrieve the stride, in bytes, of a single vertex using this format.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt16 cgVertexFormat::getStride( )
{
    // Make sure stride is up-to-date.
    if ( mElementsDirty )
        buildUsageData();
    return mStride;
}

//-----------------------------------------------------------------------------
//  Name : getStride ()
/// <summary>
/// Retrieve the stride, in bytes, of a single vertex using this format.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexFormat::isReadOnly( ) const
{
    return mInDatabase;
}

//-----------------------------------------------------------------------------
//  Name : getDeclaratorTypeSize ()
/// <summary>
/// Determine the size, in bytes, of the input data based on the
/// specified declarator type.
/// </summary>
//-----------------------------------------------------------------------------
cgUInt32 cgVertexFormat::getDeclaratorTypeSize( D3DDECLTYPE Type )
{
    switch ( Type )
    {
        case D3DDECLTYPE_D3DCOLOR:    
        case D3DDECLTYPE_FLOAT1:
        case D3DDECLTYPE_UBYTE4:
        case D3DDECLTYPE_SHORT2:
        case D3DDECLTYPE_UBYTE4N:
        case D3DDECLTYPE_SHORT2N:
        case D3DDECLTYPE_USHORT2N:
        case D3DDECLTYPE_FLOAT16_2:
        case D3DDECLTYPE_UDEC3:     // 30bits (+2 for alignment)
        case D3DDECLTYPE_DEC3N:     // 30bits (+2 for alignment)
            return 4;

        case D3DDECLTYPE_FLOAT2:
        case D3DDECLTYPE_SHORT4:
        case D3DDECLTYPE_SHORT4N:
        case D3DDECLTYPE_USHORT4N:
        case D3DDECLTYPE_FLOAT16_4:
            return 8;

        case D3DDECLTYPE_FLOAT3:
            return 12;
        
        case D3DDECLTYPE_FLOAT4:
            return 16;
    
    } // End Switch

    // Invalid type specified
    return 0;
}

//-----------------------------------------------------------------------------
//  Name : convertVertex ()
/// <summary>
/// Convert the data stored in the source vertex to the correct format
/// for storing in the destination vertex.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexFormat::convertVertex( cgByte * pDst, cgVertexFormat * pDstFormat, cgByte * pSrc, cgVertexFormat * pSrcFormat )
{
    // Validate parameters.
    if ( !pDst || !pSrc || !pDstFormat || !pSrcFormat )
        return false;

    // formats match?
    if ( pDstFormat == pSrcFormat )
    {
        memcpy( pDst, pSrc, pDstFormat->getStride() );
        return true;
    
    } // End if match

    // Update source usage map if necessary (should never be dirty
    // if it's in the vertex format database).
    if ( pSrcFormat->mElementsDirty && !pSrcFormat->mInDatabase )
        pSrcFormat->buildUsageData();

    // Iterate through the destination vertex and populate with any available data from the source.
    for ( cgUInt16 i = 0; i < pDstFormat->mElementCount; ++i )
    {
        D3DVERTEXELEMENT9 * pDstElement = &pDstFormat->mElements[i];
        D3DVERTEXELEMENT9 * pSrcElement = pSrcFormat->mUsageMap[ pDstElement->Usage + (pDstElement->UsageIndex * (MaxDeclElementIndex + 1)) ];

        // Element must exist in both, and in this version the format of the data must also match.
        if ( !pSrcElement || (pSrcElement->Type != pDstElement->Type) )
            continue;

        // Compute the size of the element data.
        cgUInt32 nSize = getDeclaratorTypeSize( (D3DDECLTYPE)pDstElement->Type );
        if ( !nSize ) continue;

        // Copy element data.
        memcpy( pDst + pDstElement->Offset, pSrc + pSrcElement->Offset, nSize );

    } // Next Element

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : convertVertices ()
/// <summary>
/// Convert the data stored in the source vertex array to the correct format
/// for storing in the destination vertex array.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexFormat::convertVertices( cgByte * pDst, cgVertexFormat * pDstFormat, cgByte * pSrc, cgVertexFormat * pSrcFormat, cgUInt32 nVertexCount )
{
    // Validate parameters.
    if ( !pDst || !pSrc || !pDstFormat || !pSrcFormat )
        return false;

    // formats match?
    if ( *pDstFormat == *pSrcFormat )
    {
        memcpy( pDst, pSrc, nVertexCount * pDstFormat->getStride() );
        return true;
    
    } // End if match

    // Extract useful information.
    cgInt nDstStride = (cgInt)pDstFormat->getStride();
    cgInt nSrcStride = (cgInt)pSrcFormat->getStride();

    // Update source usage map if necessary (should never be dirty
    // if it's in the vertex format database).
    if ( pSrcFormat->mElementsDirty && !pSrcFormat->mInDatabase )
        pSrcFormat->buildUsageData();

    // Iterate through the destination vertex and populate with any available data from the source.
    for ( cgUInt16 i = 0; i < pDstFormat->mElementCount; ++i )
    {
        D3DVERTEXELEMENT9 * pDstElement = &pDstFormat->mElements[i];
        D3DVERTEXELEMENT9 * pSrcElement = pSrcFormat->mUsageMap[ pDstElement->Usage + (pDstElement->UsageIndex * (MaxDeclElementIndex + 1)) ];

        // Element must exist in both, and in this version the format of the data must also match.
        if ( !pSrcElement || (pSrcElement->Type != pDstElement->Type) )
            continue;

        // Compute the size of the element data.
        cgUInt32 nSize = getDeclaratorTypeSize( (D3DDECLTYPE)pDstElement->Type );
        if ( !nSize ) continue;

        // Copy element data for each vertex.
        for ( cgUInt32 i = 0; i < nVertexCount; ++i )
        {
            memcpy( pDst + (i * nDstStride) + pDstElement->Offset, 
                    pSrc + (i * nSrcStride) + pSrcElement->Offset, 
                    nSize );
        
        } // Next Vertex

    } // Next Element

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : compareVertices ()
/// <summary>
/// Compare the data stored in the specified vertices to ensure that all
/// vertex components that exist matches.
/// Note : All parameters must be non-NULL.
/// </summary>
//-----------------------------------------------------------------------------
int cgVertexFormat::compareVertices( cgByte * pVtx1, cgByte * pVtx2, cgVertexFormat * pFormat, cgFloat fEpsilon )
{
    cgFloat fDifference;
    int   nDifference;

    // TODO: Test that this works properly!

    // Compare any components that exist within the vertex.
    for ( cgUInt16 i = 0; i < pFormat->mElementCount; ++i )
    {
        D3DVERTEXELEMENT9 * pElement = &pFormat->mElements[i];

        // Retrieve the vertex data pointers
        cgByte * p1 = pVtx1 + pElement->Offset;
        cgByte * p2 = pVtx2 + pElement->Offset;

        // Perform comparison based on element data type.
        switch ( pElement->Type )
        {
            case D3DDECLTYPE_FLOAT4:    
                fDifference = *((cgFloat*)p1) - *((cgFloat*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                
                // Drop through to next case
                p1 += sizeof(cgFloat);
                p2 += sizeof(cgFloat);

            case D3DDECLTYPE_FLOAT3:
                fDifference = *((cgFloat*)p1) - *((cgFloat*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                
                // Drop through to next case
                p1 += sizeof(cgFloat);
                p2 += sizeof(cgFloat);

            case D3DDECLTYPE_FLOAT2:
                fDifference = *((cgFloat*)p1) - *((cgFloat*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                
                // Drop through to next case
                p1 += sizeof(cgFloat);
                p2 += sizeof(cgFloat);

            case D3DDECLTYPE_FLOAT1:
                fDifference = *((cgFloat*)p1) - *((cgFloat*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                break;
            
            case D3DDECLTYPE_UBYTE4N:
            case D3DDECLTYPE_D3DCOLOR:
            {
                cgColorValue Color1( *((cgUInt32*)p1) );
                cgColorValue Color2( *((cgUInt32*)p2) );
                fDifference = Color1.r - Color2.r;
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                fDifference = Color1.g - Color2.g;
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                fDifference = Color1.b - Color2.b;
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                fDifference = Color1.a - Color2.a;
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                break;

            } // End case D3DDECLTYPE_D3DCOLOR
            
            case D3DDECLTYPE_UBYTE4:
            case D3DDECLTYPE_SHORT2:
                nDifference = memcmp( p1, p2, 4 );
                if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
                break;
            
            case D3DDECLTYPE_SHORT4:
                nDifference = memcmp( p1, p2, 8 );
                if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
                break;

            case D3DDECLTYPE_SHORT4N:
                fDifference = (((cgFloat)*((cgInt16*)p1)) / 32767.0f) - (((cgFloat)*((cgInt16*)p2)) / 32767.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                p1 += sizeof(cgInt16);
                p2 += sizeof(cgInt16);
                fDifference = (((cgFloat)*((cgInt16*)p1)) / 32767.0f) - (((cgFloat)*((cgInt16*)p2)) / 32767.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;

                // Drop through to next case
                p1 += sizeof(cgInt16);
                p2 += sizeof(cgInt16);

            case D3DDECLTYPE_SHORT2N:
                fDifference = (((cgFloat)*((cgInt16*)p1)) / 32767.0f) - (((cgFloat)*((cgInt16*)p2)) / 32767.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                p1 += sizeof(cgInt16);
                p2 += sizeof(cgInt16);
                fDifference = (((cgFloat)*((cgInt16*)p1)) / 32767.0f) - (((cgFloat)*((cgInt16*)p2)) / 32767.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                break;

            case D3DDECLTYPE_USHORT4N:
                fDifference = (((cgFloat)*((cgUInt16*)p1)) / 65535.0f) - (((cgFloat)*((cgUInt16*)p2)) / 65535.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                p1 += sizeof(cgUInt16);
                p2 += sizeof(cgUInt16);
                fDifference = (((cgFloat)*((cgUInt16*)p1)) / 65535.0f) - (((cgFloat)*((cgUInt16*)p2)) / 65535.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;

                // Drop through to next case
                p1 += sizeof(cgUInt16);
                p2 += sizeof(cgUInt16);

            case D3DDECLTYPE_USHORT2N:
                fDifference = (((cgFloat)*((cgUInt16*)p1)) / 65535.0f) - (((cgFloat)*((cgUInt16*)p2)) / 65535.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                p1 += sizeof(cgUInt16);
                p2 += sizeof(cgUInt16);
                fDifference = (((cgFloat)*((cgUInt16*)p1)) / 65535.0f) - (((cgFloat)*((cgUInt16*)p2)) / 65535.0f);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                break;

            case D3DDECLTYPE_UDEC3:
            {
                cgUInt32 n1 = *(cgUInt32*)p1, n2 = *(cgUInt32*)p2;
                nDifference = (int)((n1 & 0xFFC00000) >> 22) - (int)((n2 & 0xFFC00000) >> 22);
                if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
                nDifference = (int)((n1 & 0x3FF000) >> 12) - (int)((n2 & 0x3FF000) >> 12);
                if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
                nDifference = (int)((n1 & 0xFFC) >> 2) - (int)((n2 & 0xFFC) >> 2);
                if ( nDifference != 0 ) return (nDifference < 0) ? -1 : 1;
                break;

            } // End case D3DDECLTYPE_UDEC3

            case D3DDECLTYPE_DEC3N:
            {
                // ToDo: Currently unsupported

            } // End case D3DDECLTYPE_DEC3N
            case D3DDECLTYPE_FLOAT16_4:
                fDifference = (cgFloat)*((D3DXFLOAT16*)p1) - (cgFloat)*((D3DXFLOAT16*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                p1 += sizeof(D3DXFLOAT16);
                p2 += sizeof(D3DXFLOAT16);
                fDifference = (cgFloat)*((D3DXFLOAT16*)p1) - (cgFloat)*((D3DXFLOAT16*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                
                // Drop through to next case
                p1 += sizeof(D3DXFLOAT16);
                p2 += sizeof(D3DXFLOAT16);
            
            case D3DDECLTYPE_FLOAT16_2:
                fDifference = (cgFloat)*((D3DXFLOAT16*)p1) - (cgFloat)*((D3DXFLOAT16*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                p1 += sizeof(D3DXFLOAT16);
                p2 += sizeof(D3DXFLOAT16);
                fDifference = (cgFloat)*((D3DXFLOAT16*)p1) - (cgFloat)*((D3DXFLOAT16*)p2);
                if ( fabsf( fDifference ) > fEpsilon ) return (fDifference < 0) ? -1 : 1;
                break;

        } // End Switch Type

    } // Next Element

    // Both vertices are equal for the purposes of this test.
    return 0;

}

//-----------------------------------------------------------------------------
//  Name : buildUsageData () (Private)
/// <summary>
/// Once the elements have been filled out, this function builds a look up 
/// table that accelerates the retrieval of those elements. The stride is also
/// precomputed here.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::buildUsageData( )
{
    // Bail if there is nothing to do.
    if ( !mElementsDirty )
        return;

    // Allocate the the usage map if it isn't already available
    if ( !mUsageMap )
        mUsageMap = new D3DVERTEXELEMENT9*[ 14 * (MaxDeclElementIndex+1) ];

    // Clear the existing data (we'll fill it again).
    memset( mUsageMap, 0, sizeof(D3DVERTEXELEMENT9*) * 14 * (MaxDeclElementIndex+1) );

    // Iterate through all declared elements and set them in the map.
    for ( cgUInt32 i = 0; i < mElementCount; ++i )
    {
        // Skip if this will put us out of range
        if ( mElements[i].UsageIndex > MaxDeclElementIndex )
            continue;

        // Store
        mUsageMap[ mElements[i].Usage + (mElements[i].UsageIndex * (MaxDeclElementIndex + 1)) ] = &mElements[i];

    } // Next Element

    // Recompute the stride from this declarator (stream 0)
    mStride = D3DXGetDeclVertexSize( mElements, 0 );

    // Elements are no longer dirty
    mElementsDirty = false;

}

//-----------------------------------------------------------------------------
//  Name : addVertexElement ()
/// <summary>
/// Allows for sequential construction of the elements in the vertex
/// format. Note: appending a vertex element will fail if the format is already 
/// contained in the format database (isReadOnly()).
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexFormat::addVertexElement( cgUInt8 Type, cgUInt8 Usage )
{
    // Pass through to full method with an index of 0
    return addVertexElement( Type, Usage, 0 );
}

//-----------------------------------------------------------------------------
//  Name : addVertexElement ()
/// <summary>
/// Allows for sequential construction of the elements in the vertex
/// format. Note: appending a vertex element will fail if the format is already 
/// contained in the format database (isReadOnly()).
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexFormat::addVertexElement( cgUInt8 Type, cgUInt8 Usage, cgUInt8 Index )
{
    // Cannot add further elements if this format exists in the database.
    if ( isReadOnly() )
        return false;

    // Populate the new element structure
    D3DVERTEXELEMENT9 NewElement;
    NewElement.Stream     = 0;
    NewElement.Type       = Type;
    NewElement.Method     = 0;
    NewElement.Usage      = Usage;
    NewElement.UsageIndex = Index;
    NewElement.Offset     = 0;
    
    // Allocate enough room for the new element (remember, we always allocate 1 more than necessary
    // for the 'D3DDECL_END' element.
    D3DVERTEXELEMENT9 * pNewElements = new D3DVERTEXELEMENT9[ mElementCount + 2 ];

    // Compute the correct offset and then copy over the old elements if there were any
    if ( mElementCount > 0 )
    {
        // Compute the offset of this new element based on the offset and length of the last element in the list
        NewElement.Offset = mElements[mElementCount-1].Offset + (cgUInt16)getDeclaratorTypeSize( (D3DDECLTYPE)mElements[mElementCount-1].Type );

        // Copy existing data.
        memcpy( pNewElements, mElements, mElementCount * sizeof(D3DVERTEXELEMENT9) );
        delete []mElements;
    
    } // End if old elements exist

    // Update buffer pointer and add the new element
    mElements                    = pNewElements;
    mElements[ mElementCount ] = NewElement;
    mElementCount++;

    // Add the end / terminating element
    D3DVERTEXELEMENT9 EndElement = D3DDECL_END();
    mElements[ mElementCount ] = EndElement;

    // Elements are dirty. Make sure they get recomputed on the next request.
    mElementsDirty = true;

    // Element added.
    return true;
}


//-----------------------------------------------------------------------------
//  Name : operator= ()
/// <summary>
/// Assignment operator, makes a deep copy of the specified object and
/// stores them in the current format object.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat & cgVertexFormat::operator=( const cgVertexFormat & VertexFormat )
{
    // Cannot assign if this format is read only.
    if ( isReadOnly() )
        return *this;

    // Clear internal array
    delete []mElements;
    mElements = CG_NULL;
    
    // Copy data
    mElementCount = VertexFormat.mElementCount;
    mStride       = VertexFormat.mStride;

    // Allocate new elements
    if ( mElementCount > 0 )
    {
        D3DVERTEXELEMENT9 EndElement = D3DDECL_END();
        mElements = new D3DVERTEXELEMENT9[ mElementCount + 1 ];
        memcpy( mElements, VertexFormat.mElements, mElementCount * sizeof(D3DVERTEXELEMENT9) );
        mElements[ mElementCount ] = EndElement;

    } // End if any elements

    // Rebuild our internal usage data if necessary
    mElementsDirty = VertexFormat.mElementsDirty;
    if ( !mElementsDirty && VertexFormat.mUsageMap )
    {
        // Allocate the the usage map if it isn't already available
        // and then duplicate from the source format.
        if ( !mUsageMap )
            mUsageMap = new D3DVERTEXELEMENT9*[ 14 * (MaxDeclElementIndex+1) ];
        memcpy( mUsageMap, VertexFormat.mUsageMap, sizeof( D3DVERTEXELEMENT9*) * 14 * (MaxDeclElementIndex+1) );

    } // End if duplicate usage
    else
    {
        buildUsageData();
    
    } // End if rebuild usage

    // Return reference to self
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator== ()
/// <summary>
/// 'Equal' operator, tests for equality with another format object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexFormat::operator==( const cgVertexFormat & VertexFormat ) const
{
    // Perform early out compares
    if ( mElementCount != VertexFormat.mElementCount )
        return false;
    if ( (mElements && !VertexFormat.mElements) || (!mElements && VertexFormat.mElements) )
        return false;
    if ( !mElements && !VertexFormat.mElements )
        return true;

    // Compare stride as a quick early-out test if both formats are up-to-date.
    if ( !mElementsDirty && !VertexFormat.mElementsDirty && mStride != VertexFormat.mStride )
        return false;
    
    // Perform final memory compare
    return ( memcmp( mElements, VertexFormat.mElements, mElementCount * sizeof(D3DVERTEXELEMENT9) ) == 0 );
}

//-----------------------------------------------------------------------------
//  Name : operator!= ()
/// <summary>
/// 'Not Equal' operator, tests for inequality with another format object.
/// </summary>
//-----------------------------------------------------------------------------
bool cgVertexFormat::operator!=( const cgVertexFormat & VertexFormat ) const
{
    // Perform early out compares
    if ( mElementCount != VertexFormat.mElementCount )
        return true;
    if ( (mElements && !VertexFormat.mElements) || (!mElements && VertexFormat.mElements) )
        return true;
    if ( !mElements && !VertexFormat.mElements )
        return false;

    // Compare stride as a quick early-out test if both formats are up-to-date.
    if ( !mElementsDirty && !VertexFormat.mElementsDirty && mStride != VertexFormat.mStride )
        return true;
    
    // Perform final memory compare
    return ( memcmp( mElements, VertexFormat.mElements, mElementCount * sizeof(D3DVERTEXELEMENT9) ) != 0 );

} // End operator!=

//-----------------------------------------------------------------------------
//  Name : getDefaultFormat () (Static)
/// <summary>
/// Retrieve the actual format structure for the current default format.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat * cgVertexFormat::getDefaultFormat( )
{
    return mDefaultFormat;
}

//-----------------------------------------------------------------------------
//  Name : setDefaultFormat () (Static)
/// <summary>
/// Set the default vertex format that we should use.
/// </summary>
//-----------------------------------------------------------------------------
void cgVertexFormat::setDefaultFormat( cgVertexFormat * pFormat )
{
    mDefaultFormat = pFormat;
}

//-----------------------------------------------------------------------------
//  Name : formatFromFVF () (Static)
/// <summary>
/// Generate a new format from the specified FVF.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat * cgVertexFormat::formatFromFVF( cgUInt32 nFVF )
{
    D3DVERTEXELEMENT9 Elements[ MAX_FVF_DECL_SIZE ];
    
    // Generate a set of declarator elements from the FVF specified
    if ( FAILED( D3DXDeclaratorFromFVF( nFVF, Elements ) ) )
        return CG_NULL;

    // Pass through to the formatFromDeclarator function
    return formatFromDeclarator( Elements );
}

//-----------------------------------------------------------------------------
//  Name : formatFromDeclarator () (Static)
/// <summary>
/// Generate a new format from the specified declarator
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat * cgVertexFormat::formatFromDeclarator( const D3DVERTEXELEMENT9 Declarator[] )
{
    cgUInt16 i, nElementCount = 0;

    // Calculate the stride from this declarator (stream 0). If it
    // fails then the declarator is invalid.
    cgUInt32 nStride = D3DXGetDeclVertexSize( Declarator, 0 );
    if ( !nStride )
        return CG_NULL;

    // First compute the number of elements in this declarator
    for ( i = 0; i < MAX_FVF_DECL_SIZE; ++i )
    {
        if ( Declarator[i].Stream == 0xFF )
            break;
        nElementCount++;
    
    } // Next Element

    // If we reached the end this is most likely an error
    if ( nElementCount == MAX_FVF_DECL_SIZE )
        return CG_NULL;

    // Allocate a new format for us to populate
    cgVertexFormat * pNewFormat = new cgVertexFormat;

    // Allocate the correct number of elements (valid elements + 1 end element)
    pNewFormat->mElements = new D3DVERTEXELEMENT9[ nElementCount + 1 ];

    // Copy over the element data (includng the declaration 'end' item)
    memcpy( pNewFormat->mElements, Declarator, (nElementCount + 1) * sizeof(D3DVERTEXELEMENT9) );
    pNewFormat->mElementCount  = nElementCount;
    
    // Setup stride and mark as not dirty (initially) in order to activate 
    // the format inequality acceleration relating to strides.
    pNewFormat->mStride        = (cgUInt16)nStride;
    pNewFormat->mElementsDirty = false;

    // Format already exists?
    cgVertexFormat * pExistingFormat = getExistingFormat( pNewFormat );
    if ( pExistingFormat )
    {
        // ToDo: 9999 - ScriptSafeDispose() when this becomes a script item.
        delete pNewFormat;
        return pExistingFormat;
    
    } // End if already exists

    // Complete preparation
    pNewFormat->mElementsDirty = true;
    pNewFormat->mInDatabase = true;
    pNewFormat->buildUsageData();
    
    // Store the new format and return it
    mFormats.formats.push_back( pNewFormat );
    return pNewFormat;
}

//-----------------------------------------------------------------------------
//  Name : getExistingFormat () (Private,Static)
/// <summary>
/// Determine if a matching format already exists in the table.
/// </summary>
//-----------------------------------------------------------------------------
cgVertexFormat * cgVertexFormat::getExistingFormat( cgVertexFormat * pFormat )
{
    cgUInt32 i;
    FormatDatabase::FormatArray & Formats = mFormats.formats;

    // Loop through all formats
    for ( i = 0; i < Formats.size(); ++i )
    {
        // Test with 'cgVertexFormat' provided operator overload
        if ( *Formats[i] == *pFormat )
            return Formats[i];
    
    } // Next vertex format

    // Nothing found
    return CG_NULL;
}

///////////////////////////////////////////////////////////////////////////////
// cgVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgVertex::textureCoords definition
const int cgVertex::TextureCoordCount = 4;

// cgVertex::Declarator definition
const D3DVERTEXELEMENT9 cgVertex::Declarator[9] =
{
    // cgVector3 position;
    {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
    // cgVector3 normal;
    {0,12,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_NORMAL,0},
    // cgVector3 binormal;
    {0,24,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_BINORMAL,0},
    // cgVector3 tangent;
    {0,36,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_TANGENT,0},
    // cgVector2 textureCoords[4];
    {0,48,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
    {0,56,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,1},
    {0,64,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,2},
    {0,72,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,3},
    D3DDECL_END()
};

//-----------------------------------------------------------------------------
//  Name : cgVertex () (Constructors)
/// <summary>
/// cgVertex Class Constructors
/// </summary>
//-----------------------------------------------------------------------------
cgVertex::cgVertex( ) 
{ 
	position = normal = binormal = tangent = cgVector3( 0, 0, 0 ); 
	memset( textureCoords, 0, TextureCoordCount * sizeof(cgVector2) ); 
}

cgVertex::cgVertex( cgFloat fX, cgFloat fY, cgFloat fZ, const cgVector3 & vecNormal, cgFloat ftu, cgFloat ftv ) 
{ 
	position.x  = fX;
    position.y  = fY;
    position.z  = fZ; 
	normal      = vecNormal; 
	binormal    = tangent = cgVector3( 0, 0, 0 ); 
	
    // Copy tex coords
    memset( textureCoords, 0, TextureCoordCount * sizeof(cgVector2) ); 
	textureCoords[ 0 ].x = ftu; 
	textureCoords[ 0 ].y = ftv; 
}

cgVertex::cgVertex( const cgVector3 & vecPos, const cgVector3 & vecNormal, cgFloat ftu, cgFloat ftv ) 
{ 
	position    = vecPos; 
	normal      = vecNormal; 
	binormal    = tangent = cgVector3( 0, 0, 0 ); 

    // Copy tex coords
	memset( textureCoords, 0, TextureCoordCount * sizeof(cgVector2) ); 
	textureCoords[ 0 ].x = ftu; 
	textureCoords[ 0 ].y = ftv; 
}

///////////////////////////////////////////////////////////////////////////////
// cgScreenVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgScreenVertex::Declarator definition
const D3DVERTEXELEMENT9 cgScreenVertex::Declarator[4] =
{
    // cgVector4 position;
    {0,0,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_POSITIONT,0},
    // cgUInt32 Color;
    {0,16,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
    // cgVector2 textureCoords;
    {0,20,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
    D3DDECL_END()
};

///////////////////////////////////////////////////////////////////////////////
// cgShadedVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgShadedVertex::Declarator definition
const D3DVERTEXELEMENT9 cgShadedVertex::Declarator[3] =
{
    // cgVector3 position;
    {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
    // cgUInt32 Color;
    {0,12,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
    D3DDECL_END()
};

///////////////////////////////////////////////////////////////////////////////
// cgClipVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgClipVertex::Declarator definition
const D3DVERTEXELEMENT9 cgClipVertex::Declarator[3] =
{
    // cgVector3 position;
    {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
    // cgVector2 textureCoords;
    {0,12,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
    D3DDECL_END()
};

//-----------------------------------------------------------------------------
//  Name : cgClipVertex () (Constructors)
/// <summary>
/// cgClipVertex Class Constructors
/// </summary>
//-----------------------------------------------------------------------------
cgClipVertex::cgClipVertex( ) 
{ 
	position  = cgVector3( 0, 0, 0 ); 
    textureCoords = cgVector2( 0, 0 );
}

cgClipVertex::cgClipVertex( cgFloat fX, cgFloat fY, cgFloat fZ, cgFloat ftu, cgFloat ftv ) 
{ 
	position.x  = fX; 
    position.y  = fY;
    position.z  = fZ;
	textureCoords.x = ftu;
    textureCoords.y = ftv; 
}

cgClipVertex::cgClipVertex( const cgVector3 & vecPos, cgFloat ftu, cgFloat ftv ) 
{ 
	position    = vecPos; 
	textureCoords.x = ftu; 
	textureCoords.y = ftv; 
}

///////////////////////////////////////////////////////////////////////////////
// cgBillboard3DVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgBillboard3DVertex::Declarator definition
const D3DVERTEXELEMENT9 cgBillboard3DVertex::Declarator[7] =
{
    // cgVector3 position;
    {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
    // cgUInt32 Color;
    {0,12,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
    // cgVector2 UV;
    {0,16,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
    // cgVector2 Offset;
    {0,24,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,1},
    // cgFloat   Angle;
    // cgVector2 Scale;
    {0,32,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_TEXCOORD,2},
    // cgVector3 Direction;
    {0,44,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_TEXCOORD,3},
    D3DDECL_END()
};

///////////////////////////////////////////////////////////////////////////////
// cgBillboard2DVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgBillboard2DVertex::Declarator definition
const D3DVERTEXELEMENT9 cgBillboard2DVertex::Declarator[4] =
{
    // cgVector4 position;
    {0,0,D3DDECLTYPE_FLOAT4,0,D3DDECLUSAGE_POSITIONT,0},
    // cgUInt32 Color;
    {0,16,D3DDECLTYPE_D3DCOLOR,0,D3DDECLUSAGE_COLOR,0},
    // cgVector2 UV;
    {0,20,D3DDECLTYPE_FLOAT2,0,D3DDECLUSAGE_TEXCOORD,0},
    D3DDECL_END()
};

///////////////////////////////////////////////////////////////////////////////
// cgPointVertex Member Definitions
///////////////////////////////////////////////////////////////////////////////

// cgPointVertex::Declarator definition
const D3DVERTEXELEMENT9 cgPointVertex::Declarator[2] =
{
    // cgVector3 position;
    {0,0,D3DDECLTYPE_FLOAT3,0,D3DDECLUSAGE_POSITION,0},
    D3DDECL_END()
};