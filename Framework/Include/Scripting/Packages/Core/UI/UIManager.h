#pragma once

// Required headers
#include <Scripting/cgScriptPackage.h>
#include <Interface/cgUIManager.h>

// Parent hierarchy
namespace cgScriptPackages { namespace Core { namespace UI {

// Package declaration
namespace UIManager
{

    // Package descriptor
    class Package : public cgScriptPackage
    {
        BEGIN_SCRIPT_PACKAGE( "Core.UI.UIManager" )
        END_SCRIPT_PACKAGE( )

        // Type declarations
        void declare( cgScriptEngine * engine )
        {
            BINDSUCCESS( engine->registerObjectType( "UIManager", 0, asOBJ_REF ) );
        }

        // Member bindings
        void bind( cgScriptEngine * engine )
        {
            using namespace cgScriptInterop::Utils;

            // Register the reference/object handle support for the objects.
            registerHandleBehaviors<cgUIManager>( engine );

            // Register base class methods
            Core::System::References::Reference::registerReferenceMethods<cgUIManager>( engine, "UIManager" );

            // Register the object methods
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void reset()", asMETHODPR(cgUIManager, reset, (), void), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool begin()", asMETHODPR(cgUIManager, begin, (), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "ResourceManager@+ getResourceManager( )", asMETHODPR(cgUIManager,getResourceManager,(),cgResourceManager*), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "RenderDriver@+ getRenderDriver( )", asMETHODPR(cgUIManager,getRenderDriver,(),cgRenderDriver*), asCALL_THISCALL) );
            // ToDo: cgTextEngine          * GetTextEngine       ( ) { return m_pTextEngine; }

            // Interface Skins
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool addSkin( InputStream )", asMETHODPR(cgUIManager, addSkin, ( cgInputStream ), bool), asCALL_THISCALL) );
            // ToDo: cgUISkin              * GetSkinDefinition   ( const cgString & strSkinName );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool selectSkin( const String &in )", asMETHODPR(cgUIManager, selectSkin, ( const cgString& ), bool), asCALL_THISCALL) );
            // ToDo: cgUISkin              * GetCurrentSkin      ( ) { return m_pCurrentSkin; }
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "String getSkinGlyphLibrary( ) const", asMETHODPR(cgUIManager, getSkinGlyphLibrary, ( ) const, cgString), asCALL_THISCALL) );

            // Text Handling
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "String addFont( InputStream )", asMETHODPR(cgUIManager, addFont, ( cgInputStream ), cgString), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "UIManager", "String addFont( const String &in )", asFUNCTIONPR(addFont, ( const cgString&, cgUIManager* ), cgString), asCALL_CDECL_OBJLAST ) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool selectFont( const String &in )", asMETHODPR(cgUIManager, selectFont, ( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool selectDefaultFont( )", asMETHODPR(cgUIManager, selectDefaultFont, ( ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool setDefaultFont( const String &in )", asMETHODPR(cgUIManager, setDefaultFont, ( const cgString& ), bool), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "const String & getDefaultFont( ) const", asMETHODPR(cgUIManager, getDefaultFont, ( ) const, const cgString& ), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( int, int, const String&in) const", asMETHODPR(cgUIManager, printText, ( cgInt32, cgInt32, const cgString& ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( int, int, const String&in, uint) const", asMETHODPR(cgUIManager, printText, ( cgInt32, cgInt32, const cgString&, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( int, int, const String&in, uint, uint) const", asMETHODPR(cgUIManager, printText, ( cgInt32, cgInt32, const cgString&, cgUInt32, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( int, int, const String&in, uint, uint, int) const", asMETHODPR(cgUIManager, printText, ( cgInt32, cgInt32, const cgString&, cgUInt32, cgUInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( int, int, const String&in, uint, uint, int, int) const", asMETHODPR(cgUIManager, printText, ( cgInt32, cgInt32, const cgString&, cgUInt32, cgUInt32, cgInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Point &in, const String&in) const", asMETHODPR(cgUIManager, printText, ( const cgPoint&, const cgString& ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Point &in, const String&in, uint) const", asMETHODPR(cgUIManager, printText, ( const cgPoint&, const cgString&, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Point &in, const String&in, uint, uint) const", asMETHODPR(cgUIManager, printText, ( const cgPoint&, const cgString&, cgUInt32, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Point &in, const String&in, uint, uint, int) const", asMETHODPR(cgUIManager, printText, ( const cgPoint&, const cgString&, cgUInt32, cgUInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Point &in, const String&in, uint, uint, int, int) const", asMETHODPR(cgUIManager, printText, ( const cgPoint&, const cgString&, cgUInt32, cgUInt32, cgInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString& ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, uint) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, uint, uint) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, cgUInt32, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, uint, uint, int) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, cgUInt32, cgUInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, uint, uint, int, int) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, cgUInt32, cgUInt32, cgInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, const Point &in) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, const cgPoint& ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, const Point &in, uint) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, const cgPoint&, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, const Point &in, uint, uint) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, const cgPoint&, cgUInt32, cgUInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, const Point &in, uint, uint, int) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, const cgPoint&, cgUInt32, cgUInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Rect printText( const Rect &in, const String&in, const Point &in, uint, uint, int, int) const", asMETHODPR(cgUIManager, printText, ( const cgRect&, const cgString&, const cgPoint&, cgUInt32, cgUInt32, cgInt32, cgInt32 ), cgRect ), asCALL_THISCALL) );
            
            // Layers
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void addLayer( UILayer@+ )", asMETHODPR(cgUIManager, addLayer, ( cgUILayer* ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void removeLayer( UILayer@+, bool )", asMETHODPR(cgUIManager, removeLayer, ( cgUILayer*, bool ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void bringLayerToFront( UILayer@+ )", asMETHODPR(cgUIManager, bringLayerToFront, ( cgUILayer* ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void sendLayerToBack( UILayer@+ )", asMETHODPR(cgUIManager, sendLayerToBack, ( cgUILayer* ), void ), asCALL_THISCALL) );

            // Forms and controls
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void setCapture( UIControl@+ )", asMETHODPR(cgUIManager, setCapture, ( cgUIControl* ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "UIControl@+ getCapture( )", asMETHODPR(cgUIManager, getCapture, ( ), cgUIControl* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void setFocus( UIControl@+ )", asMETHODPR(cgUIManager, setFocus, ( cgUIControl* ), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "UIControl@+ getFocus( )", asMETHODPR(cgUIManager, getFocus, ( ), cgUIControl* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Form@+ createForm( const String &in, const String &in)", asMETHODPR(cgUIManager, createForm, ( const cgString&, const cgString&), cgUIForm* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Form@+ loadForm( const InputStream &in, const String &in, bool)", asMETHODPR(cgUIManager, loadForm, ( const cgInputStream&, const cgString&, bool), cgUIForm* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Form@+ loadForm( const String &in, const String &in, bool)", asFUNCTIONPR(managerLoadForm, ( const cgString&, const cgString&, bool, cgUIManager*), cgUIForm* ), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Form@+ loadForm( const InputStream &in, const String &in)", asMETHODPR(cgUIManager, loadForm, ( const cgInputStream&, const cgString&), cgUIForm* ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Form@+ loadForm( const String &in, const String &in)", asFUNCTIONPR(managerLoadForm, ( const cgString&, const cgString&, cgUIManager*), cgUIForm* ), asCALL_CDECL_OBJLAST) );

            // Images / Glyphs / Icons
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool addImage( const InputStream &in, const String &in )", asMETHODPR(cgUIManager, addImage, ( const cgInputStream&, const cgString&), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool addImage( const String &in, const String &in )", asFUNCTIONPR(addImage, ( const cgString&, const cgString&, cgUIManager*), bool ), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool addImageLibrary( const InputStream &in, const String &in )", asMETHODPR(cgUIManager, addImageLibrary, ( const cgInputStream&, const cgString&), bool ), asCALL_THISCALL) );
			BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool addImageLibrary( const String &in, const String &in )", asFUNCTIONPR(addImageLibrary, ( const cgString&, const cgString&, cgUIManager*), bool ), asCALL_CDECL_OBJLAST) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool removeImageLibrary( const String &in )", asMETHODPR(cgUIManager, removeImageLibrary, ( const cgString&), bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "bool isImageLibraryLoaded( const String &in ) const", asMETHODPR(cgUIManager, isImageLibraryLoaded, ( const cgString&) const, bool ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Size getImageSize( const String &in ) const", asMETHODPR(cgUIManager, getImageSize, ( const cgString&) const, cgSize ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "Size getImageSize( const String &in, const String &in ) const", asMETHODPR(cgUIManager, getImageSize, ( const cgString&, const cgString&) const, cgSize ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const Rect &in, const String &in )", asMETHODPR(cgUIManager, drawImage, ( const cgRect&, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const Rect &in, const String &in, const String &in )", asMETHODPR(cgUIManager, drawImage, ( const cgRect&, const cgString&, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const Rect &in, const String &in, const String &in, const ColorValue &in )", asMETHODPR(cgUIManager, drawImage, ( const cgRect&, const cgString&, const cgString&, const cgColorValue&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const Rect &in, const String &in, const String &in, const ColorValue &in, bool )", asMETHODPR(cgUIManager, drawImage, ( const cgRect&, const cgString&, const cgString&, const cgColorValue&, bool), void ), asCALL_THISCALL) );
            
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const RectF &in, const String &in )", asMETHODPR(cgUIManager, drawImage, ( const cgRectF&, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const RectF &in, const String &in, const String &in )", asMETHODPR(cgUIManager, drawImage, ( const cgRectF&, const cgString&, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const RectF &in, const String &in, const String &in, const ColorValue &in )", asMETHODPR(cgUIManager, drawImage, ( const cgRectF&, const cgString&, const cgString&, const cgColorValue&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const RectF &in, const String &in, const String &in, const ColorValue &in, bool )", asMETHODPR(cgUIManager, drawImage, ( const cgRectF&, const cgString&, const cgString&, const cgColorValue&, bool), void ), asCALL_THISCALL) );
                    
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const Point &in, const String &in )", asMETHODPR(cgUIManager, drawImage, ( const cgPoint&, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const Point &in, const String &in, const String &in )", asMETHODPR(cgUIManager, drawImage, ( const cgPoint&, const cgString&, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( const Point &in, const String &in, const String &in, const ColorValue &in )", asMETHODPR(cgUIManager, drawImage, ( const cgPoint&, const cgString&, const cgString&, const cgColorValue&), void ), asCALL_THISCALL) );

            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( ImageScaleMode, const String &in )", asMETHODPR(cgUIManager, drawImage, ( cgImageScaleMode::Base, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( ImageScaleMode, const String &in, const String &in )", asMETHODPR(cgUIManager, drawImage, ( cgImageScaleMode::Base, const cgString&, const cgString&), void ), asCALL_THISCALL) );
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void drawImage( ImageScaleMode, const String &in, const String &in, const ColorValue &in )", asMETHODPR(cgUIManager, drawImage, ( cgImageScaleMode::Base, const cgString&, const cgString&, const cgColorValue&), void ), asCALL_THISCALL) );

            // Cursors
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void selectCursor( const String &in )", asMETHODPR(cgUIManager, selectCursor, ( const cgString&), void ), asCALL_THISCALL) );
            
            // Rendering
            BINDSUCCESS( engine->registerObjectMethod( "UIManager", "void render( )", asMETHODPR(cgUIManager, render, ( ), void ), asCALL_THISCALL) );

            ///////////////////////////////////////////////////////////////////////
            // Global Utility Functions
            ///////////////////////////////////////////////////////////////////////

            // Register singleton access.
            BINDSUCCESS( engine->registerGlobalFunction( "UIManager@+ getAppUIManager( )", asFUNCTIONPR(cgUIManager::getInstance, ( ), cgUIManager*), asCALL_CDECL) );
        }

        //---------------------------------------------------------------------
        //  Name : managerLoadForm () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// UIManager::loadForm() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static cgUIForm * managerLoadForm( const cgString & stream, const cgString & name, cgUIManager * thisPointer )
        {
            return thisPointer->loadForm( stream, name );
        }

        //---------------------------------------------------------------------
        //  Name : managerLoadForm () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// UIManager::loadForm() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static cgUIForm * managerLoadForm( const cgString & stream, const cgString & name, bool modal, cgUIManager * thisPointer )
        {
            return thisPointer->loadForm( stream, name, modal );
        }

        //---------------------------------------------------------------------
        //  Name : addImage () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// UIManager::addImage() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool addImage( const cgString & stream, const cgString & referenceName, cgUIManager * thisPointer )
        {
            return thisPointer->addImage( stream, referenceName );
        }

		//---------------------------------------------------------------------
        //  Name : addImageLibrary () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// UIManager::addImageLibrary() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static bool addImageLibrary( const cgString & stream, const cgString & referenceName, cgUIManager * thisPointer )
        {
            return thisPointer->addImageLibrary( stream, referenceName );
        }

		//---------------------------------------------------------------------
        //  Name : addFont () (Static)
        /// <summary>
        /// Provides an alternative overload for the script accessible
        /// UIManager::addFont() method that allows the script to pass a
        /// string type directly (no implicit cast is supported to the required
        /// InputStream type).
        /// </summary>
        //---------------------------------------------------------------------
        static cgString addFont( const cgString & stream, cgUIManager * thisPointer )
        {
            return thisPointer->addFont( stream );
        }

    }; // End Class : Package

} } } } // End Namespace : cgScriptPackages::Core::UI::UIManager
