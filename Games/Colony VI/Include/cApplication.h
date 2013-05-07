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
// Name : cApplication.h                                                     //
//                                                                           //
// Desc : Game application class. This class is the central hub for all of   //
//        the main application processing. While no game specific logic is   //
//        contained in this class, it is primarily responsible for           //
//        initializing all of the systems that the application will rely on  //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2013 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#if !defined( _CAPPLICATION_H_ )
#define _CAPPLICATION_H_

//-----------------------------------------------------------------------------
// cApplication Header Includes
//-----------------------------------------------------------------------------
#include <Carbon.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Name : cApplication (Class)
// Desc : Central application handling class. Initializes the app and handles
//        all core processes.
//-----------------------------------------------------------------------------
class cApplication : public cgApplication
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
             cApplication();
    virtual ~cApplication();

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides cgApplication)
    //-------------------------------------------------------------------------
    virtual bool        initInstance            ( const cgString & commandLine );

protected:
    //-------------------------------------------------------------------------
    // Protected Virtual Methods (Overrides cgApplication)
    //-------------------------------------------------------------------------
    virtual bool        initApplication         ( );
    virtual void        frameRender             ( );
    virtual void        frameEnd                ( );

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
};

#endif // !_CAPPLICATION_H_