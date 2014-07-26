//---------------------------------------------------------------------------//
//         ____           _                   _____                          //
//        / ___|__ _ _ __| |__   ___  _ __   |  ___|__  _ __ __ _  ___       //
//       | |   / _` | '__| '_ \ / _ \| '_ \  | |_ / _ \| '__/ _` |/ _ \      //
//       | |__| (_| | |  | |_) | (_) | | | | |  _| (_) | | | (_| |  __/      //
//        \____\__,_|_|  |_.__/ \___/|_| |_| |_|  \___/|_|  \__, |\___|      //
//                   Game Institute - Carbon Engine Sandbox |___/            //
//                                                                           //
//---------------------------------------------------------------------------//
//                                                                           //
// File: cfRenderViewport.h                                                  //
//                                                                           //
// Desc: Contains classes responsible for drawing a representation of the    //
//       scene to a specific area of the main view. For instance, one render //
//       viewport might be responsible for rendering an orthographic top     //
//       down representation in wireframe, while another renders a full      //
//       shaded perspective view.                                            //
//                                                                           //
//---------------------------------------------------------------------------//
//        Copyright 1997 - 2012 Game Institute. All Rights Reserved.         //
//---------------------------------------------------------------------------//

#pragma once

//-----------------------------------------------------------------------------
// cfRenderViewport Header Includes
//-----------------------------------------------------------------------------
#include "cfCommon.h"

// CGE Includes
#include <Rendering/cgRenderingTypes.h>
#include <Resources/cgResourceHandles.h>
#include <World/cgWorldTypes.h>

//-----------------------------------------------------------------------------
// Global Forward Declarations
//-----------------------------------------------------------------------------
class cgCameraNode;
class cgScene;
class cgRenderView;
class cgBillboardBuffer;

namespace CarbonForge
{
    //-------------------------------------------------------------------------
    // Namespace Promotion
    //-------------------------------------------------------------------------
    using namespace System::Windows::Forms;

    //-------------------------------------------------------------------------
    // Forward Declarations
    //-------------------------------------------------------------------------
    ref class cfUISceneView;
    ref class ObjectNodeDragData;
    class cfWorldDoc;
    
    //-------------------------------------------------------------------------
    // Main Class Declarations
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    // Name : cfRenderViewport (Class)
    /// <summary>
    /// Responsible for rendering one individual viewpoint of the active scene
    /// information.
    /// </summary>
    //-------------------------------------------------------------------------
    class CARBONFORGE_API cfRenderViewport
	{
	public:
        //---------------------------------------------------------------------
        // Typedefs, Structures & Enumerators
        //---------------------------------------------------------------------
        /// <summary>Specifies the type of viewport (top, user, perspective etc.)</summary>
        enum ViewportType
        {
            Viewport_Top,
            Viewport_Bottom,
            Viewport_Left,
            Viewport_Right,
            Viewport_Front,
            Viewport_Back,
            Viewport_User,
            Viewport_Perspective,
            Viewport_Camera

        }; // End Enum ViewportType

        /// <summary>Specifies the approach that will be taken to render the objects.</summary>
        enum RenderMode
        {
            Render_WireFrame,
            Render_FlatShaded,
            Render_SmoothShaded,
            Render_FullBright
        
        }; // End Enum RenderMode

        //---------------------------------------------------------------------
        // Constructors & Destructors
        //---------------------------------------------------------------------
         cfRenderViewport( gcrootx<cfUISceneView^> oView, cgInt32 nViewportID  );
        ~cfRenderViewport();

        //---------------------------------------------------------------------
        // Public Methods
        //---------------------------------------------------------------------
        void                    Reset                           ( );
        void                    Draw                            ( );
        void                    Draw                            ( bool bRepresentOnly );
        void                    OnMouseDown                     ( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers );
        void                    OnMouseUp                       ( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers );
        void                    OnMouseMove                     ( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers );
        void                    OnMouseWheel                    ( cgUInt8 nButtons, const cgPoint & Position, cgUInt32 nModifiers, cgInt32 nDelta );
        bool                    OnSetCursor                     ( );
        void                    OnObjectNodeDrop                ( gcrootx<ObjectNodeDragData^> Data, const cgPoint & Position );
        bool                    IsActive                        ( ) const;
        void                    SetActive                       ( bool bActive );
        bool                    IsEnabled                       ( ) const;
        void                    SetEnabled                      ( bool bEnabled );
        bool                    IsCaptureLocked                 ( ) const;
        void                    SetCaptureLocked                ( bool bLocked );
        cgProjectionMode::Base  GetProjectionMode               ( ) const;
        void                    SetProjectionMode               ( cgProjectionMode::Base Mode );
        ViewportType            GetViewportType                 ( ) const;
        void                    SetViewportType                 ( ViewportType Type );
        RenderMode              GetRenderMode                   ( ) const;
        void                    SetRenderMode                   ( RenderMode Mode );
        const D3DXCOLOR       & GetBackgroundColor              ( ) const;
        void                    SetBackgroundColor              ( const D3DXCOLOR & Color );
        const D3DXCOLOR       & GetBorderColor                  ( ) const;
        void                    SetBorderColor                  ( const D3DXCOLOR & Color );
        const D3DXCOLOR       & GetActiveBorderColor            ( ) const;
        void                    SetActiveBorderColor            ( const D3DXCOLOR & Color );
        const cgRect          & GetViewportArea                 ( ) const;
        const cgRect          & GetLabelArea                    ( ) const;
        void                    SetViewportArea                 ( const cgRect & Area );
        cgRect                  GetInteriorArea                 ( ) const;
        cgViewport              GetViewport                     ( ) const;
        cgString                GetLabel                        ( ) const;
        cgPlane                 GetGridPlane                    ( ) const;
        const cgMatrix        & GetToViewMatrix                 ( ) const;
        const cgMatrix        & GetFromViewMatrix               ( ) const;
        gcrootx<cfUISceneView^> GetParentView                   ( ) const;
        cfWorldDoc            * GetParentDocument               ( ) const;
        cgScene               * GetParentScene                  ( ) const;
        cgCameraNode          * GetCamera                       ( ) const;
        void                    SetGridOffset                   ( cgFloat value );
        cgFloat                 GetGridOffset                   ( ) const;
        bool                    IsGridVisible                   ( ) const;
        void                    SetGridVisible                  ( bool bVisible );
        bool                    ShouldOrbitSelected             ( ) const;
        void                    SetOrbitSelected                ( bool bOrbit );
        bool                    ViewportToWorld                 ( const cgPoint & ptViewport, cgVector3 & vWorld );
        bool                    ViewportToWorld                 ( const cgPoint & ptViewport, const cgPlane & Plane, cgVector3 & vWorld );
        bool                    ViewportToRay                   ( const cgPoint & ptViewport, cgVector3 & vOrigin, cgVector3 & vDir );
        bool                    ViewportToMajorAxis             ( const cgPoint & ptViewport, const cgVector3 & vOrigin, cgVector3 & vWorldPos, cgVector3 & vMajorAxis );
        bool                    ViewportToMajorAxis             ( const cgPoint & ptViewport, const cgVector3 & vOrigin, const cgVector3 & vNormal, cgVector3 & vWorldPos, cgVector3 & vMajorAxis );
        bool                    WorldToViewport                 ( const cgVector3 & vWorld, cgVector3 & vViewport, bool bClipX = true, bool bClipY = true, bool bClipZ = true );
        cgFloat                 EstimateZoomFactor              ( );
        cgFloat                 EstimateZoomFactor              ( const cgVector3 & vPos );
        void                    ShowSceneSelectionProperties    ( );
        void                    RefreshSceneSelectionProperties ( );
        
    protected:
        //---------------------------------------------------------------------
        // Protected Typedefs
        //---------------------------------------------------------------------
        CGE_MAP_DECLARE( cgString, cgBillboardBuffer*, IconBufferMap )

        //---------------------------------------------------------------------
        // Protected Methods
        //---------------------------------------------------------------------
        void    SwitchCameraMode    ( cgProjectionMode::Base OldProjection, ViewportType OldType );
        void    DrawFrame           ( );
        void    DrawOverlay         ( );
        void    DrawGrid            ( );
        void    DrawObjectIcons     ( cgCameraNode * pCamera );
        void    DrawSelectedAABBs   ( );
        void    DrawSceneAABB       ( );
        void    DrawSceneCellAABBs  ( );
        void    BuildViewConvert    ( );
        void    WrapCursor          ( cgPoint & oldPos, cgPoint & newPos, gcrootx<Screen^> oWrapScreen );
        void    ProcessInput        ( );
        //void    BuildGridBuffer ( );

        //---------------------------------------------------------------------
        // Protected Variables
        //---------------------------------------------------------------------
        /// <summary>The parent scene view object into which we will render.</summary>
        gcptr(cfUISceneView^)   m_ParentView;
        /// <summary>Unique (to the parent scene view) numeric identifier for this viewport.</summary>
        cgUInt32                m_nViewportID;
        /// <summary>The Carbon render view specifically associated with this viewport.
        cgRenderView          * m_pRenderView;
        /// <summary>Specifies the projection mode of the viewport (orthgraphic / perspective)</summary>
        cgProjectionMode::Base  m_Projection;
        /// <summary>Specifies the type of viewport (top, user, perspective etc.)</summary>
        ViewportType            m_Type;
        /// <summary>Specifies the approach that will be taken to render the objects.</summary>
        RenderMode              m_RenderMode;
        /// <summary>Color with which to render the main viewport background.</summary>
        D3DXCOLOR               m_BackgroundColor;
        /// <summary>Color with which to render the main viewport border.</summary>
        D3DXCOLOR               m_BorderColor;
        /// <summary>Color with which to render the "activated" viewport border.</summary>
        D3DXCOLOR               m_ActiveBorderColor;
        /// <summary>Boolean which indicates whether or not this viewport is currently active.</summary>
        bool                    m_bActive;
        /// <summary>Boolean that indicates whether or not this viewport is currently enabled (visible).</summary>
        bool                    m_bEnabled;
        /// <summary>When true, prevents the parent view from releasing the capture of this viewport until a later time.</summary>
        bool                    m_bLockCapture;
        /// <summary>Camera currently being used to render this viewport.</summary>
        cgCameraNode          * m_pCamera;
        /// <summary>The area reserved for this viewport in the parent scene view window.</summary>
        cgRect                  m_ViewportArea;
        /// <summary>The area in this viewport reserved for the viewport display label.</summary>
        cgRect                  m_LabelArea;
        /// <summary>The cursor position at the point when this viewport was captured.</summary>
        cgPoint                 m_CapturedCursorPos;
        /// <summary>Position of cursor recorded during the previous MouseMove event.</summary>
        cgPoint                 m_PreviousCursorPos;
        /// <summary>Matrix which will convert an XZ plane based vector to view space.</summary>
        cgMatrix                m_mtxToView;
        /// <summary>Matrix which will convert a view based vector to XZ plane based.</summary>
        cgMatrix                m_mtxFromView;
        /// <summary>The monitor/screen used to wrap the cursor (multi-monitor support)</summary>
        gcrootx<Screen^>        m_oWrapScreen;
        /// <summary>Is the cursor currently hidden?.</summary>
        bool                    m_bCursorHidden;
        /// <summary>Should the grid be rendered in any scene viewport?</summary>
        bool                    m_bGridVisible;
        /// <summary>Amount to offset grid from the origin.</summary>
        cgFloat                 m_fGridOffset;
        /// <summary>Surface shader file containing techniques for drawing the UI.</summary>
        cgSurfaceShaderHandle   m_hUIShader;
        /// <summary>When camera is orbiting, it should orbit center of currently selected object(s) AABB.</summary>
        bool                    m_bOrbitSelected;
        /// <summary>Billboard buffers used to represent object icons in the sandbox viewports.</summary>
        IconBufferMap           m_IconBuffers;
    
    }; // End Class cfRenderViewport

} // End Namespace CarbonForge