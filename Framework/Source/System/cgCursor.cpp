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
// Name : cgCursor.cpp                                                       //
//                                                                           //
// Desc : Base cursor class that provides an interface between the engine    //
//        and any platform specific cursor management logic.                 //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgCursor Module Includes
//-----------------------------------------------------------------------------
#include <System/cgCursor.h>
#include <System/Platform/cgWinCursor.h>

///////////////////////////////////////////////////////////////////////////////
// cgCursor Member Definitions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : cgCursor () (Constructor)
/// <summary>
/// cgCursor Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgCursor::cgCursor( ) : cgReference( 0 )
{
}

//-----------------------------------------------------------------------------
//  Name : cgCursor () (Constructor)
/// <summary>
/// cgCursor Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgCursor::cgCursor( const cgPoint & hotSpot, const cgRect & source, const cgImage & image )  : cgReference( 0 )
{
}

//-----------------------------------------------------------------------------
//  Name : ~cgCursor () (Destructor)
/// <summary>
/// cgCursor Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgCursor::~cgCursor()
{
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object.
/// </summary>
//-----------------------------------------------------------------------------
cgCursor * cgCursor::createInstance()
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinCursor();
    
    } // End Switch platform
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : createInstance () (Static)
/// <summary>
/// Create an instance of this object, initialized using the specified image.
/// </summary>
//-----------------------------------------------------------------------------
cgCursor * cgCursor::createInstance( const cgPoint & hotSpot, const cgRect & source, const cgImage & image )
{
    // Determine which type we should create.
    const CGEConfig & Config = cgGetEngineConfig();
    switch ( Config.platform )
    {
        case cgPlatform::Windows:
            return new cgWinCursor( hotSpot, source, image );
    
    } // End Switch platform
    return CG_NULL;
}

//-----------------------------------------------------------------------------
//  Name : queryReferenceType ()
/// <summary>
/// Allows the application to determine if the inheritance hierarchy 
/// supports a particular interface.
/// </summary>
//-----------------------------------------------------------------------------
bool cgCursor::queryReferenceType( const cgUID & type ) const
{
    // Supports this interface?
    if ( type == RTID_Cursor )
        return true;

    // Unsupported.
    return false;
}