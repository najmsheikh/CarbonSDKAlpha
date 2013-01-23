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
// Name : cgRenderingCapabilities.h                                          //
//                                                                           //
// Desc : Base interface through which rendering capabilities can be queried //
//        Tested capabilities include supported buffer formats, shader model //
//        support, format filtering support and so on.                       //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once
#if !defined(_CGE_CGRENDERINGCAPABILITIES_H_)
#define _CGE_CGRENDERINGCAPABILITIES_H_

//-----------------------------------------------------------------------------
// cgRenderingCapabilities Header Includes
//-----------------------------------------------------------------------------
#include <cgBase.h>
#include <Scripting/cgScriptInterop.h>
#include <Resources/cgBufferFormatEnum.h>
#include <Rendering/cgRenderingTypes.h>

//-----------------------------------------------------------------------------
// Forward Declarations
//-----------------------------------------------------------------------------
class cgRenderDriver;

//-----------------------------------------------------------------------------
// Main Class Definitions
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgRenderingCapabilities (Class)
/// <summary>
/// Base interface through which rendering capabilities can be queried. Tested 
/// capabilities include supported buffer formats, shader model support, format
/// filtering support and so on.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgRenderingCapabilities : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgRenderingCapabilities, "RenderingCapabilities" )

public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cgRenderingCapabilities( cgRenderDriver * driver );
    virtual ~cgRenderingCapabilities( );

    //-------------------------------------------------------------------------
    // Public Static Functions
    //-------------------------------------------------------------------------
    static cgRenderingCapabilities* createInstance          ( cgRenderDriver * driver );

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    const cgBufferFormatEnum      & getBufferFormats        ( ) const;

    //-------------------------------------------------------------------------
    // Public Virtual Methods
    //-------------------------------------------------------------------------
    virtual bool                    enumerate					( ) = 0;
    virtual cgUInt32                getMaxBlendTransforms		( ) const = 0;
    virtual cgUInt32                getMaxAnisotropySamples		( ) const = 0;
    virtual bool                    supportsFastStencilFill		( ) const = 0;
    virtual bool                    supportsNonPow2Textures		( ) const = 0;
	virtual bool                    supportsDepthStencilReading ( ) const = 0;
    virtual bool                    supportsShaderModel         ( cgShaderModel::Base model ) const = 0;
    virtual bool                    getDisplayModes             ( cgDisplayMode::Array & modes ) const = 0;

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose                 ( bool disposeBase );

    //-------------------------------------------------------------------------
    // Public Inline Methods
    //-------------------------------------------------------------------------
    inline cgUInt32 getBufferFormatCaps( cgBufferType::Base bufferType, cgBufferFormat::Base format )
    {
        cgAssert( mBufferFormats != CG_NULL );
        return mBufferFormats->getFormatCaps( bufferType, format );
    }

protected:
    //-------------------------------------------------------------------------
    // Protected Member Variables
    //-------------------------------------------------------------------------
    cgRenderDriver    * mDriver;        // Driver on which we're reporting hardware capabilities.
    cgBufferFormatEnum* mBufferFormats; // Enumerated buffer formats supported by the hardware.
};

#endif // !_CGE_CGRENDERINGCAPABILITIES_H_