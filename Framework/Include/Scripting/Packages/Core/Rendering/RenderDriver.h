#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Rendering/cgRenderDriver.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Rendering {

// Package declaration
namespace RenderDriver
{
    // Namespace promotion (within parent)
    using namespace cgScriptInterop::Types;

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Rendering.RenderDriver" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "RenderDriverConfig", sizeof(cgRenderDriverConfig), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "RenderDriver", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgRenderDriverConfig (Struct)
            ///////////////////////////////////////////////////////////////////////
            
            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgRenderDriverConfig>( engine, "RenderDriverConfig" );
            
            // Register properties
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "String deviceName"      , offsetof(cgRenderDriverConfig,deviceName) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool windowed"          , offsetof(cgRenderDriverConfig,windowed) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "int width"              , offsetof(cgRenderDriverConfig,width) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "int height"             , offsetof(cgRenderDriverConfig,height) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "int refreshRate"        , offsetof(cgRenderDriverConfig,refreshRate) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool useHardwareTnL"    , offsetof(cgRenderDriverConfig,useHardwareTnL) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool useVSync"          , offsetof(cgRenderDriverConfig,useVSync) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool useTripleBuffering", offsetof(cgRenderDriverConfig,useTripleBuffering) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool debugVShader"      , offsetof(cgRenderDriverConfig,debugVShader) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool debugPShader"      , offsetof(cgRenderDriverConfig,debugPShader) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool usePerfHUD"        , offsetof(cgRenderDriverConfig,usePerfHUD) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "bool useVTFBlending"    , offsetof(cgRenderDriverConfig,useVTFBlending) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "int shadingQuality"     , offsetof(cgRenderDriverConfig,shadingQuality) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "int postProcessQuality" , offsetof(cgRenderDriverConfig,postProcessQuality) ) );
            BINDSUCCESS( engine->registerObjectProperty( "RenderDriverConfig", "int antiAliasingQuality", offsetof(cgRenderDriverConfig,antiAliasingQuality) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgRenderDriver (Class)
            ///////////////////////////////////////////////////////////////////////

            // Register the reference/object handle support for the objects
            registerHandleBehaviors<cgRenderDriver>( engine );
            
            // Register base class object methods
            Core::System::References::Reference::registerReferenceMethods<cgRenderDriver>( engine, "RenderDriver" );

            // Register the object methods
            // Configuration and Capabilities
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "RenderDriverConfig getConfig() const", asMETHODPR(cgRenderDriver, getConfig, () const, cgRenderDriverConfig), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "ConfigResult loadConfig( const String &in )", asMETHODPR(cgRenderDriver, loadConfig, ( const cgString& ), cgConfigResult::Base), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "ConfigResult loadDefaultConfig( bool )", asMETHODPR(cgRenderDriver, loadDefaultConfig, ( bool ), cgConfigResult::Base), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool saveConfig( const String &in )", asMETHODPR(cgRenderDriver, saveConfig, ( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "ResourceManager@+ getResourceManager( ) const", asMETHODPR(cgRenderDriver,getResourceManager,() const,cgResourceManager*), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "RenderingCapabilities@+ getCapabilities( ) const", asMETHODPR(cgRenderDriver,getCapabilities,() const,cgRenderingCapabilities*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool updateAdapter( int, const DisplayMode &in, bool, bool )", asMETHODPR(cgRenderDriver,updateAdapter,( cgInt32, const cgDisplayMode&, bool, bool ),bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool updateDisplayMode( const DisplayMode &in, bool, bool )", asMETHODPR(cgRenderDriver,updateDisplayMode,( const cgDisplayMode&, bool, bool ),bool), asCALL_THISCALL) );
			
			// ToDo: BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "AppWindow@+ getWindow( ) const", asMETHODPR(cgRenderDriver, GetWindow, () const, cgAppWindow*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool useHardwareTnL( ) const", asMETHODPR(cgRenderDriver,useHardwareTnL,() const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool isWindowActive( ) const", asMETHODPR(cgRenderDriver,isWindowActive,() const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool isInitialized( ) const", asMETHODPR(cgRenderDriver,isInitialized,() const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool isWindowed( ) const", asMETHODPR(cgRenderDriver,isWindowed,() const, bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool isVSyncEnabled( ) const", asMETHODPR(cgRenderDriver,isVSyncEnabled,() const, bool), asCALL_THISCALL) );        
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "HardwareType getHardwareType( ) const", asMETHODPR(cgRenderDriver,getHardwareType,( ) const, cgHardwareType::Base), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "Size getScreenSize( ) const", asMETHODPR(cgRenderDriver,getScreenSize,() const, cgSize), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const Matrix & getWorldTransform( ) const", asMETHODPR(cgRenderDriver,getWorldTransform,( ) const, const cgMatrix&), asCALL_THISCALL) );
            // ToDo: cgScriptObject                * GetShaderInterface      ( ) const;
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void windowResized( int, int )", asMETHODPR(cgRenderDriver, windowResized, (cgInt32, cgInt32), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void releaseOwnedResources( )", asMETHODPR(cgRenderDriver, releaseOwnedResources, (), void), asCALL_THISCALL) );

            // Render related functions
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawCircle( const Vector2 &in, float, const ColorValue &in, float, uint )", asMETHODPR(cgRenderDriver,drawCircle,( const cgVector2 &, cgFloat, const cgColorValue &, cgFloat, cgUInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawCircle( const Vector2 &in, float, const ColorValue &in, float, uint, float, float )", asMETHODPR(cgRenderDriver,drawCircle,( const cgVector2 &, cgFloat, const cgColorValue &, cgFloat, cgUInt32, cgFloat, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawRectangle( const Rect &in, const ColorValue &in, bool )", asMETHODPR(cgRenderDriver,drawRectangle,( const cgRect &, const cgColorValue &, bool ), void), asCALL_THISCALL) );
            // ToDo: void                            DrawLines               ( cgVector2 Points[], cgUInt32 nLineCount, const cgColorValue & Color, bool bStripBehavior = false );
            // ToDo: void                            DrawOOBB                ( const cgBoundingBox & Bounds, cgFloat fGrowAmount, const cgMatrix & mtxObject, const cgColorValue & Color, bool bSealedEdges = false );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawScreenQuad( )", asMETHODPR(cgRenderDriver,drawScreenQuad,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawScreenQuad( const SurfaceShaderHandle &in )", asMETHODPR(cgRenderDriver,drawScreenQuad,( const cgSurfaceShaderHandle& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawScreenQuad( const SurfaceShaderHandle &in, float )", asMETHODPR(cgRenderDriver,drawScreenQuad,( const cgSurfaceShaderHandle&, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawViewportClipQuad( )", asMETHODPR(cgRenderDriver,drawViewportClipQuad,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawViewportClipQuad( const SurfaceShaderHandle &in )", asMETHODPR(cgRenderDriver,drawViewportClipQuad,( const cgSurfaceShaderHandle& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawViewportClipQuad( const SurfaceShaderHandle &in, float )", asMETHODPR(cgRenderDriver,drawViewportClipQuad,( const cgSurfaceShaderHandle&, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawClipQuad( )", asMETHODPR(cgRenderDriver,drawClipQuad,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawClipQuad( float )", asMETHODPR(cgRenderDriver,drawClipQuad,( cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawClipQuad( const SurfaceShaderHandle &in )", asMETHODPR(cgRenderDriver,drawClipQuad,( const cgSurfaceShaderHandle& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawClipQuad( const SurfaceShaderHandle &in, float )", asMETHODPR(cgRenderDriver,drawClipQuad,( const cgSurfaceShaderHandle&, cgFloat ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawClipQuad( float, bool )", asMETHODPR(cgRenderDriver,drawClipQuad,( cgFloat, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawClipQuad( const SurfaceShaderHandle &in, float, bool )", asMETHODPR(cgRenderDriver,drawClipQuad,( const cgSurfaceShaderHandle&, cgFloat, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in )", asMETHODPR(cgRenderDriver,beginTargetRender,( const cgRenderTargetHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in, const DepthStencilTargetHandle &in )", asMETHODPR(cgRenderDriver,beginTargetRender,( const cgRenderTargetHandle&, const cgDepthStencilTargetHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in, int )", asMETHODPR(cgRenderDriver,beginTargetRender,( const cgRenderTargetHandle&, cgInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in, int, const DepthStencilTargetHandle &in )", asMETHODPR(cgRenderDriver,beginTargetRender,( const cgRenderTargetHandle&, cgInt32, const cgDepthStencilTargetHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in, int, bool )", asMETHODPR(cgRenderDriver,beginTargetRender,( const cgRenderTargetHandle&, cgInt32, bool ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( RenderTargetHandle, int, bool, DepthStencilTargetHandle )", asMETHODPR(cgRenderDriver,beginTargetRender,( cgRenderTargetHandle, cgInt32, bool, cgDepthStencilTargetHandle ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( array<RenderTargetHandle>@+ )", asFUNCTIONPR(beginTargetRender,( ScriptArray*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( array<RenderTargetHandle>@+, const DepthStencilTargetHandle &in )", asFUNCTIONPR(beginTargetRender,( ScriptArray*, const cgDepthStencilTargetHandle&, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( array<RenderTargetHandle>@+, bool )", asFUNCTIONPR(beginTargetRender,( ScriptArray*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( array<RenderTargetHandle>@+, bool, const DepthStencilTargetHandle &in )", asFUNCTIONPR(beginTargetRender,( ScriptArray*, bool, const cgDepthStencilTargetHandle&, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool endTargetRender( )", asMETHODPR(cgRenderDriver,endTargetRender,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginFrame( )", asMETHODPR(cgRenderDriver,beginFrame,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginFrame( bool )", asMETHODPR(cgRenderDriver,beginFrame,( bool ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginFrame( bool, uint )", asMETHODPR(cgRenderDriver,beginFrame,( bool, cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void endFrame( )", asMETHODPR(cgRenderDriver,endFrame,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void endFrame( bool )", asMETHODPR(cgRenderDriver,endFrame,( bool ), void), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void endFrame( AppWindow@+ )", asMETHODPR(cgRenderDriver,endFrame,( cgAppWindow* ), void), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void endFrame( AppWindow@+, bool )", asMETHODPR(cgRenderDriver,endFrame,( cgAppWindow*, bool ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool clear( uint, uint, float, uint8 )", asMETHODPR(cgRenderDriver,clear,( cgUInt32, cgUInt32, cgFloat, cgUInt8 ), bool), asCALL_THISCALL) );
            // ToDo: BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool Clear( uint, Rectangle[], uint, uint, cgFloat, uint8 )", asMETHODPR(cgRenderDriver,clear,( cgUInt32, D3DRECT*, cgUInt32, cgUInt32, cgFloat, cgUInt8 ), bool), asCALL_THISCALL) );
            // ToDo: bool StretchRect( ResourceHandle & hSource, RECT * pSrcRect, ResourceHandle hDestination, RECT * pDstRect, D3DTEXTUREFILTERTYPE Filter );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool stretchRect( const TextureHandle &in, const TextureHandle &in )", asMETHODPR(cgRenderDriver,stretchRect,( const cgTextureHandle&, const cgTextureHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool stretchRect( const TextureHandle &in, const Rect &in, const TextureHandle &in, const Rect&in )", asMETHODPR(cgRenderDriver,stretchRect,( const cgTextureHandle&, const cgRect*, const cgTextureHandle&, const cgRect* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawIndexedPrimitive( PrimitiveType, int, uint, uint, uint, uint )", asMETHODPR(cgRenderDriver,drawIndexedPrimitive, ( cgPrimitiveType::Base, cgInt32, cgUInt32, cgUInt32, cgUInt32, cgUInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void drawPrimitive( PrimitiveType, uint, uint )", asMETHODPR(cgRenderDriver,drawPrimitive,( cgPrimitiveType::Base, cgUInt32, cgUInt32 ), void), asCALL_THISCALL) );

            // Application rendering states
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setMaterial( MaterialHandle, bool )", asMETHODPR(cgRenderDriver,setMaterial,( cgMaterialHandle, bool ), bool), asCALL_THISCALL) );
            // ToDo: void                            SetMaterialTerms        ( const cgMaterialTerms & Terms );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const MaterialHandle& getMaterial( ) const", asMETHODPR(cgRenderDriver,getMaterial,( ) const, const cgMaterialHandle&), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void setMaterialFilter( FilterExpression@+ )", asMETHODPR(cgRenderDriver,setMaterialFilter,( cgFilterExpression* ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void pushMaterialFilter( FilterExpression@+ )", asMETHODPR(cgRenderDriver,pushMaterialFilter,( cgFilterExpression* ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void popMaterialFilter( )", asMETHODPR(cgRenderDriver,popMaterialFilter,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool matchMaterialFilter( const MaterialHandle &in )", asMETHODPR(cgRenderDriver,matchMaterialFilter,( const cgMaterialHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setWorldTransform( const Matrix &in )", asMETHODPR(cgRenderDriver,setWorldTransform,( const cgMatrix* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushWorldTransform( const Matrix &in )", asMETHODPR(cgRenderDriver,pushWorldTransform,( const cgMatrix* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popWorldTransform( )", asMETHODPR(cgRenderDriver,popWorldTransform,( ), bool), asCALL_THISCALL) );
            // ToDo: void SetVertexBlendData      ( const cgMatrix pMatrices[], const cgMatrix pITMatrices[], cgUInt32 MatrixCount, int nMaxBlendIndex = -1 );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setCamera( CameraNode @+ )", asMETHODPR(cgRenderDriver,setCamera,( cgCameraNode* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushCamera( CameraNode @+ )", asMETHODPR(cgRenderDriver,pushCamera,( cgCameraNode* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popCamera( )", asMETHODPR(cgRenderDriver,popCamera,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "CameraNode@+ getCamera( )", asMETHODPR(cgRenderDriver,getCamera,( ), cgCameraNode* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool cameraUpdated( )", asMETHODPR(cgRenderDriver,cameraUpdated,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void setRenderPass( const String &in )", asMETHODPR(cgRenderDriver,setRenderPass,( const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void pushRenderPass( const String &in )", asMETHODPR(cgRenderDriver,pushRenderPass,( const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void popRenderPass( )", asMETHODPR(cgRenderDriver,popRenderPass,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const String & getRenderPass( ) const", asMETHODPR(cgRenderDriver,getRenderPass,( ) const, const cgString& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void setOverrideMethod( const String &in )", asMETHODPR(cgRenderDriver,setOverrideMethod,( const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void pushOverrideMethod( const String &in )", asMETHODPR(cgRenderDriver,pushOverrideMethod,( const cgString& ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void popOverrideMethod( )", asMETHODPR(cgRenderDriver,popOverrideMethod,( ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const String & getOverrideMethod( ) const", asMETHODPR(cgRenderDriver,getOverrideMethod,( ) const, const cgString& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setUserClipPlanes( array<Plane>@+ )", asFUNCTIONPR(setUserClipPlanes,( ScriptArray*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setUserClipPlanes( array<Plane>@+, bool )", asFUNCTIONPR(setUserClipPlanes,( ScriptArray*, bool, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushUserClipPlanes( array<Plane>@+ )", asFUNCTIONPR(pushUserClipPlanes,( ScriptArray*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushUserClipPlanes( array<Plane>@+, bool )", asFUNCTIONPR(pushUserClipPlanes,( ScriptArray*, bool, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popUserClipPlanes( )", asMETHODPR(cgRenderDriver,popUserClipPlanes,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setScissorRect( const Rect &in )", asMETHODPR(cgRenderDriver,setScissorRect,( const cgRect* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushScissorRect( const Rect &in )", asMETHODPR(cgRenderDriver,pushScissorRect,( const cgRect* ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popScissorRect( )", asMETHODPR(cgRenderDriver,popScissorRect,( ), bool), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setSystemState( SystemState, int )", asMETHODPR(cgRenderDriver,setSystemState,( cgSystemState::Base, cgInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "int getSystemState( SystemState )", asMETHODPR(cgRenderDriver,getSystemState,( cgSystemState::Base ), cgInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setTexture( uint, const TextureHandle &in)", asMETHODPR(cgRenderDriver,setTexture,( cgUInt32, const cgTextureHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushTexture( uint, const TextureHandle &in)", asMETHODPR(cgRenderDriver,pushTexture,( cgUInt32, const cgTextureHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popTexture( uint )", asMETHODPR(cgRenderDriver,popTexture,( cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const TextureHandle & getTexture( uint ) const", asMETHODPR(cgRenderDriver,getTexture,( cgUInt32 ) const, const cgTextureHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setSamplerState( uint, const SamplerStateHandle &in)", asMETHODPR(cgRenderDriver,setSamplerState,( cgUInt32, const cgSamplerStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushSamplerState( uint, const SamplerStateHandle &in)", asMETHODPR(cgRenderDriver,pushSamplerState,( cgUInt32, const cgSamplerStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popSamplerState( uint )", asMETHODPR(cgRenderDriver,popSamplerState,( cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const SamplerStateHandle & getSamplerState( uint ) const", asMETHODPR(cgRenderDriver,getSamplerState,( cgUInt32 ) const, const cgSamplerStateHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setDepthStencilState( const DepthStencilStateHandle &in)", asMETHODPR(cgRenderDriver,setDepthStencilState,( const cgDepthStencilStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setDepthStencilState( const DepthStencilStateHandle &in, uint )", asMETHODPR(cgRenderDriver,setDepthStencilState,( const cgDepthStencilStateHandle&, cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushDepthStencilState( const DepthStencilStateHandle &in)", asMETHODPR(cgRenderDriver,pushDepthStencilState,( const cgDepthStencilStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushDepthStencilState( const DepthStencilStateHandle &in, uint )", asMETHODPR(cgRenderDriver,pushDepthStencilState,( const cgDepthStencilStateHandle&, cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popDepthStencilState( )", asMETHODPR(cgRenderDriver,popDepthStencilState,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const DepthStencilStateHandle & getDepthStencilState( ) const", asMETHODPR(cgRenderDriver,getDepthStencilState,( ) const, const cgDepthStencilStateHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setRasterizerState( const RasterizerStateHandle &in)", asMETHODPR(cgRenderDriver,setRasterizerState,( const cgRasterizerStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushRasterizerState( const RasterizerStateHandle &in)", asMETHODPR(cgRenderDriver,pushRasterizerState,( const cgRasterizerStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popRasterizerState( )", asMETHODPR(cgRenderDriver,popRasterizerState,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const RasterizerStateHandle & getRasterizerState( ) const", asMETHODPR(cgRenderDriver,getRasterizerState,( ) const, const cgRasterizerStateHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setBlendState( const BlendStateHandle &in)", asMETHODPR(cgRenderDriver,setBlendState,( const cgBlendStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushBlendState( const BlendStateHandle &in)", asMETHODPR(cgRenderDriver,pushBlendState,( const cgBlendStateHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popBlendState( )", asMETHODPR(cgRenderDriver,popBlendState,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const BlendStateHandle & getBlendState( ) const", asMETHODPR(cgRenderDriver,getBlendState,( ) const, const cgBlendStateHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setConstantBuffer( uint, const ConstantBufferHandle &in)", asMETHODPR(cgRenderDriver,setConstantBuffer,( cgUInt32, const cgConstantBufferHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setConstantBufferAuto( const ConstantBufferHandle &in)", asMETHODPR(cgRenderDriver,setConstantBufferAuto,( const cgConstantBufferHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushConstantBuffer( uint, const ConstantBufferHandle &in)", asMETHODPR(cgRenderDriver,pushConstantBuffer,( cgUInt32, const cgConstantBufferHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popConstantBuffer( uint )", asMETHODPR(cgRenderDriver,popConstantBuffer,( cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const ConstantBufferHandle & getConstantBuffer( uint ) const", asMETHODPR(cgRenderDriver,getConstantBuffer,( cgUInt32 ) const, const cgConstantBufferHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setIndices( const IndexBufferHandle &in)", asMETHODPR(cgRenderDriver,setIndices,( const cgIndexBufferHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushIndices( const IndexBufferHandle &in)", asMETHODPR(cgRenderDriver,pushIndices,( const cgIndexBufferHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popIndices( )", asMETHODPR(cgRenderDriver,popIndices,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const IndexBufferHandle & getIndices( ) const", asMETHODPR(cgRenderDriver,getIndices,( ) const, const cgIndexBufferHandle& ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setStreamSource( uint, const VertexBufferHandle &in)", asMETHODPR(cgRenderDriver,setStreamSource,( cgUInt32, const cgVertexBufferHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushStreamSource( uint, const VertexBufferHandle &in)", asMETHODPR(cgRenderDriver,pushStreamSource,( cgUInt32, const cgVertexBufferHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popStreamSource( uint )", asMETHODPR(cgRenderDriver,popStreamSource,( cgUInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const VertexBufferHandle & getStreamSource( uint ) const", asMETHODPR(cgRenderDriver,getStreamSource,( cgUInt32 ) const, const cgVertexBufferHandle& ), asCALL_THISCALL) );        
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setVertexShader( const VertexShaderHandle &in)", asMETHODPR(cgRenderDriver,setVertexShader,( const cgVertexShaderHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushVertexShader( const VertexShaderHandle &in)", asMETHODPR(cgRenderDriver,pushVertexShader,( const cgVertexShaderHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popVertexShader( )", asMETHODPR(cgRenderDriver,popVertexShader,( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setPixelShader( const PixelShaderHandle &in)", asMETHODPR(cgRenderDriver,setPixelShader,( const cgPixelShaderHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushPixelShader( const PixelShaderHandle &in)", asMETHODPR(cgRenderDriver,pushPixelShader,( const cgPixelShaderHandle& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popPixelShader( )", asMETHODPR(cgRenderDriver,popPixelShader,( ), bool), asCALL_THISCALL) );
            // ToDo: bool                   SetVertexFormat        ( cgVertexFormat * pFormat );
            // ToDo: bool                   PushVertexFormat        ( cgVertexFormat * pFormat );
            // ToDo: bool                   PopVertexFormat         ( );
            // ToDo: cgVertexFormat * GetVertexFormat         ( ) const;
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushViewport( const Viewport &in )", asMETHODPR(cgRenderDriver,pushViewport,(const cgViewport*), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool popViewport( )", asMETHODPR(cgRenderDriver,popViewport,(), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setViewport( const Viewport &in )", asMETHODPR(cgRenderDriver,setViewport,(const cgViewport*), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "const Viewport& getViewport( ) const", asMETHODPR(cgRenderDriver,getViewport,() const, const cgViewport&), asCALL_THISCALL) );

            // Material technique processing
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginMaterialRender( )", asMETHODPR(cgRenderDriver,beginMaterialRender,(), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginMaterialRender( const String &in)", asMETHODPR(cgRenderDriver,beginMaterialRender,( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "TechniqueResult executeMaterialPass( )", asMETHODPR(cgRenderDriver,executeMaterialPass,( ), cgTechniqueResult::Base), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void endMaterialRender( )", asMETHODPR(cgRenderDriver,endMaterialRender,(), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool commitChanges( )", asMETHODPR(cgRenderDriver,commitChanges,(), bool), asCALL_THISCALL) );

            // Occlusion Queries
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "int activateQuery( )", asMETHODPR(cgRenderDriver,activateQuery,( ), cgInt32), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void deactivateQuery( int )", asMETHODPR(cgRenderDriver,deactivateQuery,( cgInt32 ), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool validQuery( int )", asMETHODPR(cgRenderDriver,validQuery,( cgInt32 ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool checkQueryResults( int, uint &inout )", asMETHODPR(cgRenderDriver,checkQueryResults,( cgInt32, cgUInt32& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool checkQueryResults( int, uint &inout, bool )", asMETHODPR(cgRenderDriver,checkQueryResults,( cgInt32, cgUInt32&, bool ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void clearQueries( )", asMETHODPR(cgRenderDriver,clearQueries,( ), void), asCALL_THISCALL) );

            // Render views
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "RenderView@+ createRenderView( const String &in, ScaleMode, const RectF&in )", asMETHODPR(cgRenderDriver,createRenderView,( const cgString&, cgScaleMode::Base, const cgRectF&),cgRenderView*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "RenderView@+ createRenderView( const String &in, const Rect&in )", asMETHODPR(cgRenderDriver,createRenderView,( const cgString&, const cgRect&),cgRenderView*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "void destroyRenderView( const String &in )", asMETHODPR(cgRenderDriver,destroyRenderView,( const cgString&),void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "RenderView@+ getRenderView( const String &in )", asMETHODPR(cgRenderDriver,getRenderView,( const cgString&),cgRenderView*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "RenderView@+ getActiveRenderView( )", asMETHODPR(cgRenderDriver,getActiveRenderView,( ),cgRenderView*), asCALL_THISCALL) );
            
            // Script 'null' handle support
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setDepthStencilState( NullHandle@ )", asFUNCTIONPR(setDepthStencilState,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setDepthStencilState( NullHandle@, uint )", asFUNCTIONPR(setDepthStencilState,( void*, cgUInt32, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushDepthStencilState( NullHandle@ )", asFUNCTIONPR(pushDepthStencilState,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushDepthStencilState( NullHandle@, uint )", asFUNCTIONPR(pushDepthStencilState,( void*, cgUInt32, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setRasterizerState( NullHandle@ )", asFUNCTIONPR(setRasterizerState,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushRasterizerState( NullHandle@ )", asFUNCTIONPR(pushRasterizerState,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setBlendState( NullHandle@ )", asFUNCTIONPR(setBlendState,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushBlendState( NullHandle@ )", asFUNCTIONPR(pushBlendState,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setTexture( uint, NullHandle@ )", asFUNCTIONPR(setTexture,( cgUInt32, void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushTexture( uint, NullHandle@ )", asFUNCTIONPR(pushTexture,( cgUInt32, void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setViewport( NullHandle@ )", asFUNCTIONPR(setViewport,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushViewport( NullHandle@ )", asFUNCTIONPR(pushViewport,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool setScissorRect( NullHandle@ )", asFUNCTIONPR(setScissorRect,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool pushScissorRect( NullHandle@ )", asFUNCTIONPR(pushScissorRect,( void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in, NullHandle@ )", asFUNCTIONPR(beginTargetRender,( const cgRenderTargetHandle&, void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in, int, NullHandle@ )", asFUNCTIONPR(beginTargetRender,( const cgRenderTargetHandle&, cgInt32, void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( const RenderTargetHandle &in, int, bool, NullHandle@ )", asFUNCTIONPR(beginTargetRender,( const cgRenderTargetHandle&, cgInt32, bool, void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( array<RenderTargetHandle>@+, NullHandle@ )", asFUNCTIONPR(beginTargetRender,( ScriptArray*, void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "RenderDriver", "bool beginTargetRender( array<RenderTargetHandle>@+, bool, NullHandle@ )", asFUNCTIONPR(beginTargetRender,( ScriptArray*, bool, void*, cgRenderDriver* ), bool), asCALL_CDECL_OBJLAST) );

            ///////////////////////////////////////////////////////////////////////
            // Global Utility Functions
            ///////////////////////////////////////////////////////////////////////

            // Register singleton access.
            BINDSUCCESS( engine->registerGlobalFunction( "RenderDriver@+ getAppRenderDriver( )", asFUNCTIONPR(cgRenderDriver::getInstance, ( ), cgRenderDriver*), asCALL_CDECL) );

        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( ScriptArray * renderTargets, cgRenderDriver *thisPointer )
        {
            cgRenderTargetHandleArray renderTargetsOut;

            // Extract resource handles from the specified array.
            cgUInt32 c = renderTargets->getSize();
            renderTargetsOut.reserve(c);
            for ( cgUInt32 i = 0; i < c; ++i )
                renderTargetsOut.push_back( *(cgRenderTargetHandle*)renderTargets->at( i ) );
                
            // Execute
            return thisPointer->beginTargetRender( renderTargetsOut );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( ScriptArray * renderTargets, bool autoUseMultiSample, cgRenderDriver *thisPointer )
        {
            cgRenderTargetHandleArray renderTargetsOut;
            
            // Extract resource handles from the specified array.
            cgUInt32 c = renderTargets->getSize();
            renderTargetsOut.reserve(c);
            for ( cgUInt32 i = 0; i < c; ++i )
                renderTargetsOut.push_back( *(cgRenderTargetHandle*)renderTargets->at( i ) );
                
            // Execute
            return thisPointer->beginTargetRender( renderTargetsOut, autoUseMultiSample );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( ScriptArray * renderTargets, const cgDepthStencilTargetHandle & depthStencil, cgRenderDriver *thisPointer )
        {
            cgRenderTargetHandleArray renderTargetsOut;

            // Extract resource handles from the specified array.
            cgUInt32 c = renderTargets->getSize();
            renderTargetsOut.reserve(c);
            for ( cgUInt32 i = 0; i < c; ++i )
                renderTargetsOut.push_back( *(cgRenderTargetHandle*)renderTargets->at( i ) );
                
            // Execute
            return thisPointer->beginTargetRender( renderTargetsOut, depthStencil );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( ScriptArray * renderTargets, bool autoUseMultiSample, const cgDepthStencilTargetHandle & depthStencil, cgRenderDriver *thisPointer )
        {
            cgRenderTargetHandleArray renderTargetsOut;
            
            // Extract resource handles from the specified array.
            cgUInt32 c = renderTargets->getSize();
            renderTargetsOut.reserve(c);
            for ( cgUInt32 i = 0; i < c; ++i )
                renderTargetsOut.push_back( *(cgRenderTargetHandle*)renderTargets->at( i ) );
                
            // Execute
            return thisPointer->beginTargetRender( renderTargetsOut, autoUseMultiSample, depthStencil );
        }

        //---------------------------------------------------------------------
        //  Name : setUserClipPlanes ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setUserClipPlanes() method that allows the script to
        /// pass a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setUserClipPlanes( ScriptArray * planes, cgRenderDriver *thisPointer )
        {
            return thisPointer->setUserClipPlanes( (cgPlane*)planes->at(0), planes->getSize() );
        }

        //---------------------------------------------------------------------
        //  Name : setUserClipPlanes ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setUserClipPlanes() method that allows the script to
        /// pass a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setUserClipPlanes( ScriptArray * planes, bool negatePlanes, cgRenderDriver *thisPointer )
        {
            return thisPointer->setUserClipPlanes( (cgPlane*)planes->at(0), planes->getSize(), negatePlanes );
        }

        //---------------------------------------------------------------------
        //  Name : pushUserClipPlanes ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushUserClipPlanes() method that allows the script to
        /// pass a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushUserClipPlanes( ScriptArray * planes, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushUserClipPlanes( (cgPlane*)planes->at(0), planes->getSize() );
        }

        //---------------------------------------------------------------------
        //  Name : pushUserClipPlanes ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushUserClipPlanes() method that allows the script to
        /// pass a templated array type directly.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushUserClipPlanes( ScriptArray * planes, bool negatePlanes, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushUserClipPlanes( (cgPlane*)planes->at(0), planes->getSize(), negatePlanes );
        }

        //---------------------------------------------------------------------
        //  Name : setDepthStencilState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setDepthStencilState() method that allows the script
        /// to pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setDepthStencilState( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->setDepthStencilState( cgDepthStencilStateHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : setDepthStencilState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setDepthStencilState() method that allows the script
        /// to pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setDepthStencilState( void*nullHandle, cgUInt32 stencilRef, cgRenderDriver *thisPointer )
        {
            return thisPointer->setDepthStencilState( cgDepthStencilStateHandle::Null, stencilRef );
        }

        //---------------------------------------------------------------------
        //  Name : pushDepthStencilState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushDepthStencilState() method that allows the script
        /// to pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushDepthStencilState( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushDepthStencilState( cgDepthStencilStateHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : pushDepthStencilState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushDepthStencilState() method that allows the script
        /// to pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushDepthStencilState( void*nullHandle, cgUInt32 stencilRef, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushDepthStencilState( cgDepthStencilStateHandle::Null, stencilRef );
        }

        //---------------------------------------------------------------------
        //  Name : setRasterizerState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setRasterizerState() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setRasterizerState( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->setRasterizerState( cgRasterizerStateHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : pushRasterizerState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushRasterizerState() method that allows the script
        /// to pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushRasterizerState( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushRasterizerState( cgRasterizerStateHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : setBlendState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setBlendState() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setBlendState( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->setBlendState( cgBlendStateHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : pushBlendState ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushBlendState() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushBlendState( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushBlendState( cgBlendStateHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : setTexture ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setTexture() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setTexture( cgUInt32 index, void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->setTexture( index, cgTextureHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : pushTexture ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushTexture() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushTexture( cgUInt32 index, void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushTexture( index, cgTextureHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : setViewport ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setViewport() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setViewport( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->setViewport( CG_NULL );
        }

        //---------------------------------------------------------------------
        //  Name : pushViewport ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushViewport() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushViewport( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushViewport( CG_NULL );
        }

        //---------------------------------------------------------------------
        //  Name : setScissorRect ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::setScissorRect() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool setScissorRect( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->setScissorRect( CG_NULL );
        }

        //---------------------------------------------------------------------
        //  Name : pushScissorRect ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::pushScissorRect() method that allows the script to
        /// pass 'null'.
        /// </summary>
        //---------------------------------------------------------------------
        static bool pushScissorRect( void*nullHandle, cgRenderDriver *thisPointer )
        {
            return thisPointer->pushScissorRect( CG_NULL );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// pass 'null' for the depth stencil target.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( const cgRenderTargetHandle & target, void*nullHandle, cgRenderDriver * thisPointer )
        {
            return thisPointer->beginTargetRender( target, cgDepthStencilTargetHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// pass 'null' for the depth stencil target.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( const cgRenderTargetHandle & target, cgInt32 cubeFace, void*nullHandle, cgRenderDriver * thisPointer )
        {
            return thisPointer->beginTargetRender( target, cubeFace, cgDepthStencilTargetHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// pass 'null' for the depth stencil target.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( const cgRenderTargetHandle & target, cgInt32 cubeFace, bool autoUseMultiSample, void*nullHandle, cgRenderDriver * thisPointer )
        {
            return thisPointer->beginTargetRender( target, cubeFace, autoUseMultiSample, cgDepthStencilTargetHandle::Null );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// pass 'null' for the depth stencil target.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( ScriptArray * renderTargets, void*nullHandle, cgRenderDriver *thisPointer )
        {
            return beginTargetRender( renderTargets, cgDepthStencilTargetHandle::Null, thisPointer );
        }

        //---------------------------------------------------------------------
        //  Name : beginTargetRender ()
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// RenderDriver::beginTargetRender() method that allows the script to
        /// pass 'null' for the depth stencil target.
        /// </summary>
        //---------------------------------------------------------------------
        static bool beginTargetRender( ScriptArray * renderTargets, bool autoUseMultiSample, void*nullHandle, cgRenderDriver *thisPointer )
        {
            return beginTargetRender( renderTargets, autoUseMultiSample, cgDepthStencilTargetHandle::Null, thisPointer );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Rendering::RenderDriver