#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Resources/cgResourceTypes.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace Resources {

// Package declaration
namespace Types
{
    //-------------------------------------------------------------------------
    // Name : RegisterResourceHandleType ()
    // Desc : Register a new resource handle type.
    //-------------------------------------------------------------------------
    template <class _handleType>
    void registerResourceHandleMethods( cgScriptEngine * engine, const cgChar * typeName, const cgChar * resourceName )
    {
        std::string sName = typeName;
        std::string sResName = resourceName;

        // Register the object constructors / destructors
        BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(constructResourceHandle<_handleType>,(_handleType*),void), asCALL_CDECL_OBJLAST) );
        BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, ("void f(" + sResName + "@+)").c_str(), asFUNCTIONPR(constructResourceHandle<_handleType>,(cgResource*,_handleType*),void), asCALL_CDECL_OBJLAST) );
        BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_CONSTRUCT, ("void f(const " + sName + "&in)").c_str(), asFUNCTIONPR(constructResourceHandle<_handleType>,(const _handleType&,_handleType*),void), asCALL_CDECL_OBJLAST) );
        BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_DESTRUCT,  "void f()", asFUNCTION(destructResourceHandle<_handleType>), asCALL_CDECL_OBJLAST) );

        // Register the object operator overloads
        BINDSUCCESS( engine->registerObjectMethod( typeName, (sName + " &opAssign(const " + sName + " &in)").c_str(), asMETHODPR(_handleType, operator=, (const _handleType&), _handleType&), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, ("bool opEquals(const " + sName + " &in) const").c_str(), asMETHODPR(_handleType, operator==, (const _handleType&) const, bool), asCALL_THISCALL) );
	    BINDSUCCESS( engine->registerObjectMethod( typeName, ("int opCmp(const " + sName + " &in) const").c_str(), asFUNCTION(compareResourceHandle<_handleType>), asCALL_CDECL_OBJFIRST) );

        // Register methods.
        BINDSUCCESS( engine->registerObjectMethod( typeName, (sResName + " @+ getResource( bool )").c_str(), asMETHODPR(_handleType, getResource, (bool), _handleType::_resType*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, (sResName + " @+ getResourceSilent()").c_str(), asMETHODPR(_handleType, getResourceSilent, (), _handleType::_resType*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, ("const " + sResName + " @+ getResourceSilent() const").c_str(), asMETHODPR(_handleType, getResourceSilent, () const, const _handleType::_resType*), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "ResourceType getResourceType() const", asMETHODPR(_handleType, getResourceType, () const, cgResourceType::Base), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, ("bool exchangeResource( " + sName + " &inout )").c_str(), asMETHODPR(_handleType, exchangeResource, ( _handleType & ), bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isResourceLost( ) const", asMETHODPR(_handleType, isResourceLost, ( ) const, bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void setResourceLost( bool ) const", asMETHODPR(_handleType, setResourceLost, ( bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void close( ) const", asMETHODPR(_handleType, close, ( ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "void close( bool ) const", asMETHODPR(_handleType, close, ( bool ), void), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isValid( ) const", asMETHODPR(_handleType, isValid, ( ) const, bool), asCALL_THISCALL) );
        BINDSUCCESS( engine->registerObjectMethod( typeName, "bool isLoaded( ) const", asMETHODPR(_handleType, isLoaded, ( ) const, bool), asCALL_THISCALL) );
    
    } // End Method registerResourceHandleMethods<>

    template <class _handleType, class _baseHandleType>
    void registerResourceHandleMethodsEx( cgScriptEngine * engine, const cgChar * typeName, const cgChar * resourceName, const cgChar * baseHandleName )
    {
        std::string sName = typeName;
        std::string sBaseName = baseHandleName;

        // Register standard methods for this handle type
        registerResourceHandleMethods<_handleType>( engine, typeName, resourceName );

        // Register implicit cast TO base type
        BINDSUCCESS( engine->registerObjectBehavior( typeName, asBEHAVE_IMPLICIT_VALUE_CAST, (sBaseName + " f()").c_str(), asFUNCTIONPR((castResourceHandle<_handleType,_baseHandleType>), (_handleType*), _baseHandleType), asCALL_CDECL_OBJLAST) );

        // Register explicit cast FROM base type
        BINDSUCCESS( engine->registerObjectBehavior( baseHandleName, asBEHAVE_VALUE_CAST, (sName + " f()").c_str(), asFUNCTIONPR((castResourceHandle<_baseHandleType,_handleType>), (_baseHandleType*), _handleType), asCALL_CDECL_OBJLAST) );
    }

    //-------------------------------------------------------------------------
    //  Name : castResourceHandle () (Static)
    /// <summary>
    /// Perform value cast from one handle type to another.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _fromType, class _toType >
    _toType castResourceHandle( _fromType *thisPointer )
    {
        return *(_toType*)thisPointer;
    }

    //-------------------------------------------------------------------------
    //  Name : constructResourceHandle () (Static)
    /// <summary>
    /// This is a wrapper for the default ResourceHandle constructor, since
    /// it is not possible to take the address of the constructor directly.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _handleType>
    void constructResourceHandle( _handleType *thisPointer )
    {
        // Use placement new to allocate which will in turn call the constructor
        new(thisPointer) _handleType();
    }

    //-------------------------------------------------------------------------
    //  Name : constructResourceHandle () (Static)
    /// <summary>
    /// This is a wrapper for an overload ResourceHandle constructor, since
    /// it is not possible to take the address of the constructor directly.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _handleType>
    void constructResourceHandle( cgResource * resource, _handleType *thisPointer )
    {
        // Use placement new to allocate which will in turn call the constructor
        new(thisPointer) _handleType( (_handleType::_resType*)resource );
    }

    //-------------------------------------------------------------------------
    //  Name : constructResourceHandle () (Static)
    /// <summary>
    /// This is a wrapper for an overload ResourceHandle constructor, since
    /// it is not possible to take the address of the constructor directly.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _handleType>
    void constructResourceHandle( const _handleType & resource, _handleType *thisPointer )
    {
        // Use placement new to allocate which will in turn call the constructor
        new(thisPointer) _handleType( resource );
    }

    //-------------------------------------------------------------------------
    //  Name : destructResourceHandle () (Static)
    /// <summary>
    /// A wrapper for the ResourceHandle destructor. This will be triggered
    /// whenever it goes out of scope inside the script.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _handleType>
    void destructResourceHandle( _handleType * thisPointer )
    {
        thisPointer->~_handleType();
    }

    //-------------------------------------------------------------------------
    //  Name : compareResourceHandle () (Static)
    /// <summary>
    /// Compare the two specified resource handles.
    /// </summary>
    //-------------------------------------------------------------------------
    template <class _handleType>
    int compareResourceHandle( const _handleType &a, const _handleType &b )
    {
	    int cmp = 0;
	    if ( a < b ) cmp = -1;
	    else if ( a > b ) cmp = 1;
	    return cmp;
    }

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.Resources.Types" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            // Value Types / Structures
            BINDSUCCESS( engine->registerObjectType( "AudioBufferFormat" , sizeof(cgAudioBufferFormat) , asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS ) );
            BINDSUCCESS( engine->registerObjectType( "ImageInfo"         , sizeof(cgImageInfo)         , asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "MaterialTerms"     , sizeof(cgMaterialTerms)     , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "ConstantDesc"      , sizeof(cgConstantDesc)      , asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            BINDSUCCESS( engine->registerObjectType( "ConstantBufferDesc", sizeof(cgConstantBufferDesc), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );

            // Enumerations
            BINDSUCCESS( engine->registerEnum( "BufferFormat" ) );
            BINDSUCCESS( engine->registerEnum( "BufferType" ) );
            BINDSUCCESS( engine->registerEnum( "BufferUsage" ) );
            BINDSUCCESS( engine->registerEnum( "LockFlags" ) );
            BINDSUCCESS( engine->registerEnum( "MemoryPool" ) );
            BINDSUCCESS( engine->registerEnum( "MultiSampleType" ) );
            BINDSUCCESS( engine->registerEnum( "ResourceType" ) );
            BINDSUCCESS( engine->registerEnum( "ResourceFlags" ) );
            BINDSUCCESS( engine->registerEnum( "AudioBufferFlags" ) );
            BINDSUCCESS( engine->registerEnum( "ResampleMethod" ) );
            BINDSUCCESS( engine->registerEnum( "ConstantType" ) );
            BINDSUCCESS( engine->registerEnum( "ScaleMode" ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            ///////////////////////////////////////////////////////////////////////
            // cgBufferFormat (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "Unknown", cgBufferFormat::Unknown ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32A32_Typeless", cgBufferFormat::R32G32B32A32_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32A32_Float", cgBufferFormat::R32G32B32A32_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32A32_UInt", cgBufferFormat::R32G32B32A32_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32A32_SInt", cgBufferFormat::R32G32B32A32_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32_Typeless", cgBufferFormat::R32G32B32_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32_Float", cgBufferFormat::R32G32B32_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32_UInt", cgBufferFormat::R32G32B32_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32B32_SInt", cgBufferFormat::R32G32B32_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16B16A16_Typeless", cgBufferFormat::R16G16B16A16_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16B16A16_Float", cgBufferFormat::R16G16B16A16_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16B16A16", cgBufferFormat::R16G16B16A16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16B16A16_UInt", cgBufferFormat::R16G16B16A16_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16B16A16_Signed", cgBufferFormat::R16G16B16A16_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16B16A16_SInt", cgBufferFormat::R16G16B16A16_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32_Typeless", cgBufferFormat::R32G32_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32_Float", cgBufferFormat::R32G32_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32_UInt", cgBufferFormat::R32G32_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G32_SInt", cgBufferFormat::R32G32_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32G8X24_Typeless", cgBufferFormat::R32G8X24_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "D32_Float_S8X24_UInt", cgBufferFormat::D32_Float_S8X24_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32_Float_X8X24_Typeless", cgBufferFormat::R32_Float_X8X24_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "X32_Typeless_G8X24_UInt", cgBufferFormat::X32_Typeless_G8X24_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R10G10B10A2_Typeless", cgBufferFormat::R10G10B10A2_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R10G10B10A2", cgBufferFormat::R10G10B10A2 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R10G10B10A2_UInt", cgBufferFormat::R10G10B10A2_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R11G11B10_Float", cgBufferFormat::R11G11B10_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8B8A8_Typeless", cgBufferFormat::R8G8B8A8_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8B8A8", cgBufferFormat::R8G8B8A8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8B8A8_SRGB", cgBufferFormat::R8G8B8A8_SRGB ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8B8A8_UInt", cgBufferFormat::R8G8B8A8_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8B8A8_Signed", cgBufferFormat::R8G8B8A8_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8B8A8_SInt", cgBufferFormat::R8G8B8A8_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16_Typeless", cgBufferFormat::R16G16_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16_Float", cgBufferFormat::R16G16_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16", cgBufferFormat::R16G16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16_UInt", cgBufferFormat::R16G16_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16_Signed", cgBufferFormat::R16G16_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16G16_SInt", cgBufferFormat::R16G16_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32_Typeless", cgBufferFormat::R32_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "D32_Float", cgBufferFormat::D32_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32_Float", cgBufferFormat::R32_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "Index32", cgBufferFormat::Index32 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32_UInt", cgBufferFormat::R32_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R32_SInt", cgBufferFormat::R32_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R24G8_Typeless", cgBufferFormat::R24G8_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "D24S8", cgBufferFormat::D24S8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "D24_UNorm_S8_UInt", cgBufferFormat::D24_UNorm_S8_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R24_UNorm_X8_Typeless", cgBufferFormat::R24_UNorm_X8_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "X24_Typeless_G8_UInt", cgBufferFormat::X24_Typeless_G8_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8_Typeless", cgBufferFormat::R8G8_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8", cgBufferFormat::R8G8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8_UInt", cgBufferFormat::R8G8_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8_Signed", cgBufferFormat::R8G8_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8_SInt", cgBufferFormat::R8G8_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16_Typeless", cgBufferFormat::R16_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16_Float", cgBufferFormat::R16_Float ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "D16", cgBufferFormat::D16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16", cgBufferFormat::R16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "Index16", cgBufferFormat::Index16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16_UInt", cgBufferFormat::R16_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16_Signed", cgBufferFormat::R16_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R16_SInt", cgBufferFormat::R16_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8_Typeless", cgBufferFormat::R8_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8", cgBufferFormat::R8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8_UInt", cgBufferFormat::R8_UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8_Signed", cgBufferFormat::R8_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8_SInt", cgBufferFormat::R8_SInt ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "A8", cgBufferFormat::A8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R1", cgBufferFormat::R1 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R9G9B9E5_SharedExp", cgBufferFormat::R9G9B9E5_SharedExp ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R8G8_B8G8", cgBufferFormat::R8G8_B8G8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "G8R8_G8B8", cgBufferFormat::G8R8_G8B8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC1_Typeless", cgBufferFormat::BC1_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC1", cgBufferFormat::BC1 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC1_SRGB", cgBufferFormat::BC1_SRGB ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC2_Typeless", cgBufferFormat::BC2_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC2", cgBufferFormat::BC2 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC2_SRGB", cgBufferFormat::BC2_SRGB ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC3_Typeless", cgBufferFormat::BC3_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC3", cgBufferFormat::BC3 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC3_SRGB", cgBufferFormat::BC3_SRGB ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC4_Typeless", cgBufferFormat::BC4_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC4", cgBufferFormat::BC4 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC4_Signed", cgBufferFormat::BC4_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC5_Typeless", cgBufferFormat::BC5_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC5", cgBufferFormat::BC5 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC5_Signed", cgBufferFormat::BC5_Signed ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B5G6R5", cgBufferFormat::B5G6R5 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B5G5R5A1", cgBufferFormat::B5G5R5A1 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B8G8R8A8", cgBufferFormat::B8G8R8A8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B8G8R8X8", cgBufferFormat::B8G8R8X8 ) );
            //BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "R10G10B10_XR_Bias_A2_UNorm", cgBufferFormat::R10G10B10_XR_Bias_A2_UNorm ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B8G8R8A8_Typeless", cgBufferFormat::B8G8R8A8_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B8G8R8A8_SRGB", cgBufferFormat::B8G8R8A8_SRGB ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B8G8R8X8_Typeless", cgBufferFormat::B8G8R8X8_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B8G8R8X8_SRGB", cgBufferFormat::B8G8R8X8_SRGB ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC6H_Typeless", cgBufferFormat::BC6H_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC6H_UF16", cgBufferFormat::BC6H_UF16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC6H_SF16", cgBufferFormat::BC6H_SF16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC7_Typeless", cgBufferFormat::BC7_Typeless ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC7", cgBufferFormat::BC7 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "BC7_SRGB", cgBufferFormat::BC7_SRGB ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "DF16", cgBufferFormat::DF16 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "DF24", cgBufferFormat::DF24 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "INTZ", cgBufferFormat::INTZ ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "RAWZ", cgBufferFormat::RAWZ ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B8G8R8", cgBufferFormat::B8G8R8 ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferFormat", "B5G5R5X1", cgBufferFormat::B5G5R5X1 ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBufferType (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "None"               , cgBufferType::None ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "Texture1D"          , cgBufferType::Texture1D ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "Texture2D"          , cgBufferType::Texture2D ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "Texture3D"          , cgBufferType::Texture3D ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "TextureCube"        , cgBufferType::TextureCube ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "Surface"            , cgBufferType::Surface ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "RenderTarget"       , cgBufferType::RenderTarget ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "RenderTargetCube"   , cgBufferType::RenderTargetCube ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "DepthStencil"       , cgBufferType::DepthStencil ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "ShadowMap"          , cgBufferType::ShadowMap ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "Video"              , cgBufferType::Video ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferType", "DeviceBackBuffer"   , cgBufferType::DeviceBackBuffer ) );

            ///////////////////////////////////////////////////////////////////////
            // cgBufferUsage (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "BufferUsage", "WriteOnly"         , cgBufferUsage::WriteOnly ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferUsage", "SoftwareProcessing", cgBufferUsage::SoftwareProcessing ) );
            BINDSUCCESS( engine->registerEnumValue( "BufferUsage", "Dynamic"           , cgBufferUsage::Dynamic ) );

            ///////////////////////////////////////////////////////////////////////
            // cgLockFlags (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "LockFlags", "ReadOnly"     , cgLockFlags::ReadOnly ) );
            BINDSUCCESS( engine->registerEnumValue( "LockFlags", "NoSystemLock" , cgLockFlags::NoSystemLock ) );
            BINDSUCCESS( engine->registerEnumValue( "LockFlags", "NoOverwrite"  , cgLockFlags::NoOverwrite ) );
            BINDSUCCESS( engine->registerEnumValue( "LockFlags", "Discard"      , cgLockFlags::Discard ) );
            BINDSUCCESS( engine->registerEnumValue( "LockFlags", "DoNotWait"    , cgLockFlags::DoNotWait ) );
            BINDSUCCESS( engine->registerEnumValue( "LockFlags", "NoDirtyUpdate", cgLockFlags::NoDirtyUpdate ) );
            BINDSUCCESS( engine->registerEnumValue( "LockFlags", "WriteOnly"    , cgLockFlags::WriteOnly ) );

            ///////////////////////////////////////////////////////////////////////
            // cgMemoryPool (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "MemoryPool", "Default"  , cgMemoryPool::Default ) );
            BINDSUCCESS( engine->registerEnumValue( "MemoryPool", "Managed"  , cgMemoryPool::Managed ) );
            BINDSUCCESS( engine->registerEnumValue( "MemoryPool", "SystemMem", cgMemoryPool::SystemMem ) );
            BINDSUCCESS( engine->registerEnumValue( "MemoryPool", "Scratch"  , cgMemoryPool::Scratch ) );

            ///////////////////////////////////////////////////////////////////////
            // cgMultiSampleType (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "None"           , cgMultiSampleType::None ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "NonMaskable"    , cgMultiSampleType::NonMaskable ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "TwoSamples"     , cgMultiSampleType::TwoSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "ThreeSamples"   , cgMultiSampleType::ThreeSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "FourSamples"    , cgMultiSampleType::FourSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "FiveSamples"    , cgMultiSampleType::FiveSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "SixSamples"     , cgMultiSampleType::SixSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "SevenSamples"   , cgMultiSampleType::SevenSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "EightSamples"   , cgMultiSampleType::EightSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "NineSamples"    , cgMultiSampleType::NineSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "TenSamples"     , cgMultiSampleType::TenSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "ElevenSamples"  , cgMultiSampleType::ElevenSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "TwelveSamples"  , cgMultiSampleType::TwelveSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "ThirteenSamples", cgMultiSampleType::ThirteenSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "FourteenSamples", cgMultiSampleType::FourteenSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "FifteenSamples" , cgMultiSampleType::FifteenSamples ) );
            BINDSUCCESS( engine->registerEnumValue( "MultiSampleType", "SixteenSamples" , cgMultiSampleType::SixteenSamples ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgResourceType (Enum)
            ///////////////////////////////////////////////////////////////////////
            
            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "None"              , cgResourceType::None ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "Texture"           , cgResourceType::Texture ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "Material"          , cgResourceType::Material ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "VertexBuffer"      , cgResourceType::VertexBuffer ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "IndexBuffer"       , cgResourceType::IndexBuffer ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "ConstantBuffer"    , cgResourceType::ConstantBuffer ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "RenderTarget"      , cgResourceType::RenderTarget ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "DepthStencilTarget", cgResourceType::DepthStencilTarget ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "AnimationSet"      , cgResourceType::AnimationSet ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "Mesh"              , cgResourceType::Mesh ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "AudioBuffer"       , cgResourceType::AudioBuffer ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "Script"            , cgResourceType::Script ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "SurfaceShader"     , cgResourceType::SurfaceShader ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "VertexShader"      , cgResourceType::VertexShader ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "PixelShader"       , cgResourceType::PixelShader ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "SamplerState"      , cgResourceType::SamplerState ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "DepthStencilState" , cgResourceType::DepthStencilState ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "RasterizerState"   , cgResourceType::RasterizerState ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceType", "BlendState"        , cgResourceType::BlendState ) );

            ///////////////////////////////////////////////////////////////////////
            // cgResourceFlags (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "ResourceFlags", "AlwaysResident", cgResourceFlags::AlwaysResident ) );
            BINDSUCCESS( engine->registerEnumValue( "ResourceFlags", "DeferredLoad"  , cgResourceFlags::DeferredLoad ) );

            ///////////////////////////////////////////////////////////////////////
            // cgAudioBufferFlags (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "Simple"     , cgAudioBufferFlags::Simple ) );
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "Streaming"  , cgAudioBufferFlags::Streaming ) );
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "Positional" , cgAudioBufferFlags::Positional ) );
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "AllowPan"   , cgAudioBufferFlags::AllowPan ) );
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "AllowVolume", cgAudioBufferFlags::AllowVolume ) );
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "AllowPitch" , cgAudioBufferFlags::AllowPitch ) );
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "Complex"    , cgAudioBufferFlags::Complex ) );
            BINDSUCCESS( engine->registerEnumValue( "AudioBufferFlags", "Complex3D"  , cgAudioBufferFlags::Complex3D ) );

            ///////////////////////////////////////////////////////////////////////
            // cgResampleMethod (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "ResampleMethod", "Average", cgResampleMethod::Average ) );
            BINDSUCCESS( engine->registerEnumValue( "ResampleMethod", "Extents", cgResampleMethod::Extents ) );

            ///////////////////////////////////////////////////////////////////////
            // cgScaleMode (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "ScaleMode", "Absolute", cgScaleMode::Absolute ) );
            BINDSUCCESS( engine->registerEnumValue( "ScaleMode", "Relative", cgScaleMode::Relative ) );

            ///////////////////////////////////////////////////////////////////////
            // cgConstantType (Enum)
            ///////////////////////////////////////////////////////////////////////

            // Register Values
            BINDSUCCESS( engine->registerEnumValue( "ConstantType", "Float"     , cgConstantType::Float ) );
            BINDSUCCESS( engine->registerEnumValue( "ConstantType", "Int"       , cgConstantType::Int ) );
            BINDSUCCESS( engine->registerEnumValue( "ConstantType", "UInt"      , cgConstantType::UInt ) );
            BINDSUCCESS( engine->registerEnumValue( "ConstantType", "Bool"      , cgConstantType::Bool ) );
            BINDSUCCESS( engine->registerEnumValue( "ConstantType", "Double"    , cgConstantType::Double ) );
            BINDSUCCESS( engine->registerEnumValue( "ConstantType", "Unresolved", cgConstantType::Unresolved ) );

            ///////////////////////////////////////////////////////////////////////
            // cgConstantDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgConstantBufferDesc>( engine, "ConstantDesc" );

            // Register Values
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "String name"                 , offsetof(cgConstantDesc,name) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "String fullTypeName"         , offsetof(cgConstantDesc,fullTypeName) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "String primitiveTypeName"    , offsetof(cgConstantDesc,primitiveTypeName) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "bool isUdt"                  , offsetof(cgConstantDesc,isUDT) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "bool isArray"                , offsetof(cgConstantDesc,isArray) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int typeId"                  , offsetof(cgConstantDesc,typeId) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "uint[] arrayDimensions"      , offsetof(cgConstantDesc,arrayDimensions) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int elements"                , offsetof(cgConstantDesc,elements) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int elementLength"           , offsetof(cgConstantDesc,elementLength) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int rows"                    , offsetof(cgConstantDesc,rows) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int columns"                 , offsetof(cgConstantDesc,columns) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int offset"                  , offsetof(cgConstantDesc,offset) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int alignment"               , offsetof(cgConstantDesc,alignment) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "int totalLength"             , offsetof(cgConstantDesc,totalLength) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "uint8[] defaultValue"        , offsetof(cgConstantDesc,defaultValue) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantDesc", "PropertyContainer parameters", offsetof(cgConstantDesc,parameters) ) );

            // Requires array type for the above single constant descriptor.
            BINDSUCCESS( engine->registerObjectType( "ConstantDesc[]", sizeof(std::vector<cgConstantDesc>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ) );
            STDVectorHelper<cgConstantDesc>::registerMethods( engine, "ConstantDesc[]", "ConstantDesc" );

            ///////////////////////////////////////////////////////////////////////
            // cgConstantBufferDesc (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgConstantBufferDesc>( engine, "ConstantBufferDesc" );

            // Register Values
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "String parentNamespace"      , offsetof(cgConstantBufferDesc,parentNamespace) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "String name"                 , offsetof(cgConstantBufferDesc,name) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "int length"                  , offsetof(cgConstantBufferDesc,length) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "int alignment"               , offsetof(cgConstantBufferDesc,alignment) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "PropertyContainer parameters", offsetof(cgConstantBufferDesc,parameters) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "ConstantDesc[] constants"    , offsetof(cgConstantBufferDesc,constants) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "int bufferRegister"          , offsetof(cgConstantBufferDesc,bufferRegister) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "int globalOffset"            , offsetof(cgConstantBufferDesc,globalOffset) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "bool bindVS"                 , offsetof(cgConstantBufferDesc,bindVS) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ConstantBufferDesc", "bool bindPS"                 , offsetof(cgConstantBufferDesc,bindPS) ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgAudioBufferFormat (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register Properties
            BINDSUCCESS( engine->registerObjectProperty( "AudioBufferFormat", "uint16 formatType"         , offsetof(cgAudioBufferFormat,formatType) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AudioBufferFormat", "uint16 channels"           , offsetof(cgAudioBufferFormat,channels) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AudioBufferFormat", "uint samplesPerSecond"     , offsetof(cgAudioBufferFormat,samplesPerSecond) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AudioBufferFormat", "uint averageBytesPerSecond", offsetof(cgAudioBufferFormat,averageBytesPerSecond) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AudioBufferFormat", "uint16 blockAlign"         , offsetof(cgAudioBufferFormat,blockAlign) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AudioBufferFormat", "uint16 bitsPerSample"      , offsetof(cgAudioBufferFormat,bitsPerSample) ) );
            BINDSUCCESS( engine->registerObjectProperty( "AudioBufferFormat", "uint16 size"               , offsetof(cgAudioBufferFormat,size) ) );

            ///////////////////////////////////////////////////////////////////////
            // cgMaterialTerms (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgMaterialTerms>( engine, "MaterialTerms" );

            // Register Properties
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "ColorValue diffuse"       , offsetof(cgMaterialTerms,diffuse) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "ColorValue ambient"       , offsetof(cgMaterialTerms,ambient) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "ColorValue specular"      , offsetof(cgMaterialTerms,specular) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "ColorValue emissive"      , offsetof(cgMaterialTerms,emissive) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float gloss"              , offsetof(cgMaterialTerms,gloss) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float emissiveHdrScale"   , offsetof(cgMaterialTerms,emissiveHDRScale) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float diffuseOpacityMapStrength"   , offsetof(cgMaterialTerms,diffuseOpacityMapStrength) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float specularOpacityMapStrength"  , offsetof(cgMaterialTerms,specularOpacityMapStrength) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float metalnessAmount"    , offsetof(cgMaterialTerms,metalnessAmount) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float metalnessDiffuse"   , offsetof(cgMaterialTerms,metalnessDiffuse) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float metalnessSpecular"  , offsetof(cgMaterialTerms,metalnessSpecular) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float rimExponent"        , offsetof(cgMaterialTerms,rimExponent) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float rimIntensity"       , offsetof(cgMaterialTerms,rimIntensity) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float reflectionIntensity", offsetof(cgMaterialTerms,reflectionIntensity) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float reflectionBumpiness", offsetof(cgMaterialTerms,reflectionBumpiness) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float reflectionMipLevel" , offsetof(cgMaterialTerms,reflectionMipLevel) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float fresnelDiffuse"      , offsetof(cgMaterialTerms,fresnelDiffuse) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float fresnelSpecular"     , offsetof(cgMaterialTerms,fresnelSpecular) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float fresnelReflection"   , offsetof(cgMaterialTerms,fresnelReflection) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float fresnelOpacity"      , offsetof(cgMaterialTerms,fresnelOpacity) ) );
            BINDSUCCESS( engine->registerObjectProperty( "MaterialTerms", "float fresnelExponent"     , offsetof(cgMaterialTerms,fresnelExponent) ) );
            
            ///////////////////////////////////////////////////////////////////////
            // cgImageInfo (Struct)
            ///////////////////////////////////////////////////////////////////////

            // Register the default constructor, destructor and assignment operators.
            registerDefaultCDA<cgImageInfo>( engine, "ImageInfo" );

            // Register Properties
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "uint width"                     , offsetof(cgImageInfo,width) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "uint height"                    , offsetof(cgImageInfo,height) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "uint depth"                     , offsetof(cgImageInfo,depth) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "uint mipLevels"                 , offsetof(cgImageInfo,mipLevels) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "BufferFormat format"            , offsetof(cgImageInfo,format) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "BufferType type"                , offsetof(cgImageInfo,type) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "MemoryPool pool"                , offsetof(cgImageInfo,pool) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "MultiSampleType multiSampleType", offsetof(cgImageInfo,multiSampleType) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "uint multiSampleQuality"        , offsetof(cgImageInfo,multiSampleQuality) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "bool autoGenerateMipmaps"       , offsetof(cgImageInfo,autoGenerateMipmaps) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "bool autoDetectFormat"          , offsetof(cgImageInfo,autoDetectFormat) ) );
            BINDSUCCESS( engine->registerObjectProperty( "ImageInfo", "bool dynamic"                   , offsetof(cgImageInfo,dynamic) ) );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::Resources::Types