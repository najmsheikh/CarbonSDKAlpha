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
// Name : cgBillboardBuffer.h                                                //
//                                                                           //
// Desc : Contains classes which provide support for both three dimensional  //
//        and two dimensional (pre-transformed), screen oriented billboards. //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGBILLBOARDBUFFER_H_)
#define _CGE_CGBILLBOARDBUFFER_H_

//-----------------------------------------------------------------------------
// cgBillboardBuffer Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Resources/cgResourceHandles.h>
#include <Math/cgMathTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;
class cgCameraNode;
class cgResourceManager;
class cgVertexFormat;
class cgBillboard;
class cgSampler;
class cgXMLNode;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgBillboardBuffer (Class)
/// <summary>
/// A container for billboards which share the same texture and effect.
/// Allows optimized drawing of all billboards in the buffer (sorted or 
/// otherwise), or in batches of an arbitrary size.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboardBuffer
{
public:
    //-------------------------------------------------------------------------
    // Public Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    CGE_UNORDEREDMAP_DECLARE(cgString, cgInt16, IndexMap)

    struct FrameDesc
    {
        cgRect      bounds;                 // The fixed point bounding rectangle of the frame in pixels
        cgRectF     textureBounds;          // The floating point rectangle describing this frame in texture space

    }; // End Struct FrameDesc
    CGE_VECTOR_DECLARE(FrameDesc, FrameArray)

    struct FrameGroup
    {
        FrameArray  frames;
        IndexMap    frameNames;             // Map containing the indices to frame groups by name

    }; // End Struct FrameGroup
    CGE_VECTOR_DECLARE(FrameGroup, FrameGroupArray)

    enum Flags
    {
        SupportSorting  = 0x1,              // It's possible to sort the billboards using renderSorted()
        ScreenSpace     = 0x2,              // Billboard buffer contains screen space (pre-transformed) billboards
        OrientationY    = 0x4,              // Billboards should be orientated about the world Y axis.
        OrientationAxis = 0x8,              // Billboards should be orientated about the arbitrary axis specified for each billboard.

        Flags_Force_32Bit = 0x7FFFFFFF

    }; // End Struct Flags

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBillboardBuffer( );
    virtual ~cgBillboardBuffer( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    bool                    prepareBuffer           ( cgUInt32 flags, cgRenderDriver * driver, cgInputStream textureFile, cgInputStream shaderFile = cgString::Empty );
    bool                    prepareBufferFromAtlas  ( cgUInt32 flags, cgRenderDriver * driver, cgInputStream atlasFile, cgInputStream shaderFile = cgString::Empty );
    cgInt32                 addBillboard            ( cgBillboard * billboard );
    bool                    buildUniformFrames      ( cgInt16 frameCount, cgInt16 framePitch );
    cgInt16                 addFrameGroup           ( );
    cgInt16                 addFrameGroup           ( const cgString & groupName );
    cgInt16                 addFrame                ( cgInt16 groupIndex, const cgRect & pixelBounds );
    cgInt16                 addFrame                ( cgInt16 groupIndex, const cgRect & pixelBounds, const cgString & frameName );
    bool                    endPrepare              ( );
    void                    render                  ( cgInt32 billboardBegin = 0, cgInt32 billboardCount = -1, bool precise = true );
    void                    renderSorted            ( cgCameraNode * camera, cgInt32 billboardBegin = 0, cgInt32 billboardCount = -1, bool precise = true );
    void                    billboardUpdated        ( cgInt32 billboardId );
    cgBillboard           * getBillboard            ( cgInt32 billboardId );
    cgUInt32                getBillboardCount       ( ) const { return (cgUInt32)mBillboards.size(); }
    void                  * getBillboardVertices    ( cgInt32 billboardId );
    const FrameDesc       * getFrameData            ( cgInt16 groupIndex, cgInt16 frameIndex ) const;
    cgInt16                 getFrameGroupIndex      ( const cgString & groupName ) const;
    cgInt16                 getFrameIndex           ( cgInt16 groupIndex, const cgString & frameName ) const;
    void                    setDirty                ( bool dirty ) { mBufferDirty = dirty; }
    void                    onDeviceLost            ( );
    void                    onDeviceReset           ( );
    void                    clear                   ( bool destroyBillboards );

private:
    //-------------------------------------------------------------------------
    // Private Typedefs, Structures and Enumerations
    //-------------------------------------------------------------------------
    // Container for storing the billboards added so far
    CGE_VECTOR_DECLARE(cgBillboard*, BillboardArray)

    // Structure used to sort billboards based on depth.
    struct DepthSortInfo
    {
        cgFloat depth;              // Squared distance from the camera (or just z component when in screen space)
        cgInt32 billboardId;        // The Id of the billboard in question.
    };

    // Cached technique handles
    struct Techniques
    {
        cgScriptFunctionHandle precise2D;
        cgScriptFunctionHandle standard2D;
        cgScriptFunctionHandle upAxisLocked3D;
        cgScriptFunctionHandle customAxisLocked3D;
        cgScriptFunctionHandle standard3D;
    };

    //-------------------------------------------------------------------------
    // Private Methods
    //-------------------------------------------------------------------------
    bool                parseAtlas              ( const cgXMLNode & mainNode );
    bool                selectTechnique         ( cgSurfaceShader * shader, bool precise = true );
    bool                restoreBuffers          ( );

    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static cgInt        billboardDepthCompare   ( const void *arg1, const void *arg2 );

    //-------------------------------------------------------------------------
    // Private Variables
    //-------------------------------------------------------------------------
    cgUInt32                mFlags;                 // The flags specified when the buffer was prepared
    cgRenderDriver        * mDriver;                // The driver used for accessing rendering features.
    cgResourceManager     * mResources;             // Main resource manager that will be used to manage the buffer's resources.
    BillboardArray          mBillboards;            // List of all billboards being managed
    cgByteArray             mSysVertices;           // The system memory vertex container used to store the billboard data.
    bool                    mBufferDirty;           // The vertex data is dirty and needs to be updated.
    cgVertexBufferHandle    mRenderVertices;        // Actual renderable vertex buffer
    cgIndexBufferHandle     mRenderIndices;         // Actual renderable index buffer
    cgIndexBufferHandle     mSortedIndices;         // If sorting is supported, contains indices to billboards sorted by distance.
    cgTextureHandle         mTexture;               // The texture assigned to the billboards in this buffer.
    cgImageInfo             mTextureInfo;           // Information about the texture assigned to the billboards.
    cgSurfaceShaderHandle   mRenderShader;          // The surface shader used to describe the rendering of these billboards.
    FrameGroupArray         mFrameGroups;           // Contains a list of grouped billboard frame data.
    IndexMap                mFrameGroupNames;       // Map containing the indices to frame groups by name
    cgSampler             * mSampler;               // The object used to bind texture / sampler information to the effect.
    cgVertexFormat        * mVertexFormat;          // Vertex format to assume for this buffer.
    Techniques              mTechniques;            // Cached technique handles.
};

//-----------------------------------------------------------------------------
//  Name : cgBillboard (Class)
/// <summary>
/// Base class responsible for managing and accessing the data within an 
/// individual billboard in the buffer.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboard
{
public:
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------
    friend cgInt32 cgBillboardBuffer::addBillboard( cgBillboard * billboard );

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBillboard( );
    virtual ~cgBillboard( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool        update          ( ) = 0;

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                setPosition     ( const cgVector3 & position );
    void                setPosition     ( cgFloat x, cgFloat y, cgFloat z );
    const cgVector3 & getPosition     ( ) const { return mPosition; }
    void                setSize         ( const cgSizeF & size );
    void                setSize         ( cgFloat width, cgFloat height );
    const cgSizeF     & getSize         ( ) const { return mSize; }
    void                setFrameGroup   ( cgInt16 groupIndex );
    void                setFrame        ( cgInt16 frameIndex, bool autoSetSize = false );
    void                setVisible      ( bool visible );
    bool                getVisible      ( ) const { return mVisible; }
    void                setColor        ( cgUInt32 color );
    cgUInt32            getColor        ( ) const { return mColor; }
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgBillboardBuffer * mBuffer;        // Our parent billboard buffer.
    cgVector3           mPosition;      // The position of the billboard (x,y,z for 3D & potentially x,y,depth for 2D)
    cgSizeF             mSize;          // The base size of the billboard
    cgInt32             mBillboardId;   // The Id of this billboard as maintained by the parent buffer
    bool                mVisible;       // Is the billboard visible or not?
    cgUInt32            mColor;         // The color of the billboard.
    cgInt16             mCurrentGroup;  // Frame group currently used for generating texture coords
    cgInt16             mCurrentFrame;  // The actual frame index within the texture for generating tex coords.
};

//-----------------------------------------------------------------------------
//  Name : cgBillboard2D (Class)
/// <summary>
/// Class responsible for managing and accessing the data within an 
/// individual 2D billboard in the buffer.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboard2D : public cgBillboard
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBillboard2D( );
    virtual ~cgBillboard2D( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool        update          ( );

};

//-----------------------------------------------------------------------------
//  Name : cgBillboard3D (Class)
/// <summary>
/// Class responsible for managing and accessing the data within an 
/// individual 3D billboard in the buffer.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgBillboard3D : public cgBillboard
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgBillboard3D( );
    virtual ~cgBillboard3D( );

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool        update          ( );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void                setScale        ( const cgVector2 & scale );
    void                setScale        ( cgFloat x, cgFloat y );
    const cgVector2 & getScale        ( ) const { return mScale; }
    void                setRotation     ( cgFloat degrees );
    cgFloat             getRotation     ( ) const { return mRotation; }
    void                setDirection    ( const cgVector3 & direction );
    void                setDirection    ( cgFloat x, cgFloat y, cgFloat z );
    const cgVector3 & getDirection    ( ) const { return mDirection; }
    
protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    cgVector2           mScale;         // The scaling factor to apply to the billboard
    cgFloat             mRotation;      // Rotation angle in degrees
    cgVector3           mDirection;     // Optional axis to which a billboard's orientation can be aligned
};

#endif // !_CGE_CGBILLBOARDBUFFER_H_