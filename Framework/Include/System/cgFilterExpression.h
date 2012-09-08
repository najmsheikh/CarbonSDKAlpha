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
// Name : cgFilterExpression.h                                               //
//                                                                           //
// Desc : A utility class that allows for complex (string based) boolean     //
//        expressions to be supplied that will be compiled/interpreted for   //
//        use in evaluations of set/unset bits in a supplied 64 bit value.   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2010 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

#pragma once
#if !defined( _CGE_CGFILTEREXPRESSION_H_ )
#define _CGE_CGFILTEREXPRESSION_H_

//-----------------------------------------------------------------------------
// cgFilterExpression Header Includes
//-----------------------------------------------------------------------------
#include <cgBaseTypes.h>
#include <Scripting/cgScriptInterop.h>

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : cgFilterExpression (Class)
/// <summary>
/// A utility class that allows for complex (string based) boolean style 
/// expressions to be supplied that will be compiled/interpreted for use in 
/// evaluations of set/unset bits in a supplied 64 bit value.
/// </summary>
//-----------------------------------------------------------------------------
class CGE_API cgFilterExpression : public cgScriptInterop::DisposableScriptObject
{
    DECLARE_SCRIPTOBJECT( cgFilterExpression, "FilterExpression" )

public:
    //-------------------------------------------------------------------------
    // Public Structures & Typedefs
    //-------------------------------------------------------------------------
    struct Identifier
    {
        cgString name;
        cgUInt64 value;
    };
    CGE_VECTOR_DECLARE( Identifier, IdentifierArray );

    //-------------------------------------------------------------------------
    // Constructors & Destructors for This Class
    //-------------------------------------------------------------------------
             cgFilterExpression();
             cgFilterExpression( const cgFilterExpression * init );
             cgFilterExpression( const cgString & expression );
             cgFilterExpression( const cgString & expression, const IdentifierArray & defines );
	virtual ~cgFilterExpression();

    //-------------------------------------------------------------------------
	// Public Methods
	//-------------------------------------------------------------------------
    bool            isCompiled          ( ) const;
    bool            compileExpression   ( const cgString & expression );
    bool            compileExpression   ( const cgString & expression, const IdentifierArray & defines );
    bool            evaluate            ( cgUInt64 value );

    //-------------------------------------------------------------------------
    // Public Virtual Methods (Overrides DisposableScriptObject)
    //-------------------------------------------------------------------------
    virtual void    dispose             ( bool disposeBase );

protected:
    //-------------------------------------------------------------------------
	// Protected Enumerations
	//-------------------------------------------------------------------------
    enum FilterTokenType
    {
        TokenUnknown,
        TokenWhitespace,
        TokenIdentifier,
        TokenAnd,
        TokenOr,
        TokenNot,
        TokenOpenParen,
        TokenCloseParen
    };

    enum FilterOpCodes
    {
        OpUnknown,

        OpSet,      // SET
        OpRSet,     // Result SET -- (result produced by pop)
        OpNSet,     // NOT SET -- (inverted set)
        OpRNSet,    // Result NOT SET

        OpAnd,      // AND
        OpRAnd,     // Result AND
        OpNAnd,     // NOT AND
        OpRNAnd,    // Result NOT AND

        OpOr,       // OR
        OpROr,      // Result OR 
        OpNOr,      // NOT OR
        OpRNOr,     // Result NOT OR

        // Early out instructions
        OpSetJz,    // SET JUMP IF ZERO
        OpRSetJz,   // Result SET JUMP IF ZERO
        OpNSetJz,   // NOT SET JUMP IF ZERO
        OpRNSetJz,  // Result NOT SET JUMP IF ZERO

        OpAndJz,    // AND JUMP IF ZERO
        OpRAndJz,   // Result AND JUMP IF ZERO
        OpNAndJz,   // NOT AND JUMP IF ZERO
        OpRNAndJz,  // Result NOT AND JUMP IF ZERO

        // Stack instructions
        OpPush,
        OpPop,

        OpUnused = 0x7FFFFFFF // Force minimum of 32 bits in size
    };

    //-------------------------------------------------------------------------
	// Protected Structures & Typedefs
	//-------------------------------------------------------------------------
    CGE_MAP_DECLARE( cgString, cgUInt64, FilterDefineMap );

    struct FilterToken
    {
        FilterTokenType     type;
        size_t              offset;
        cgString            valueName;
        cgUInt64            valueData;

        // Constructors
        FilterToken( FilterTokenType _type, size_t _offset ) :
            type( _type ), offset( _offset ), valueData( 0 ) {}
        FilterToken( FilterTokenType _type, size_t _offset, const cgString & _valueName ) :
            type( _type ), offset( _offset ), valueName( _valueName ), valueData( 0 ) {}
    };
    CGE_VECTOR_DECLARE( FilterToken, FilterTokenArray )

    struct FilterInstruction
    {
        FilterOpCodes       opCode;
        size_t              jumpCount; // If this is a 'Jz' op, and term evaluates to false, how far ahead should we jump?
        cgUInt64            bits;

        // Constructors
        FilterInstruction( ) :
            opCode( OpUnknown ), bits( 0 ), jumpCount(0) {}
        FilterInstruction( FilterOpCodes _opCode ) :
            opCode( _opCode ), bits( 0 ), jumpCount(0) {}
        FilterInstruction( FilterOpCodes _opCode, cgUInt64 _bits ) :
            opCode( _opCode ), bits( _bits ), jumpCount(0) {}
    };
    CGE_VECTOR_DECLARE( FilterInstruction, FilterInstructionArray )

    //-------------------------------------------------------------------------
	// Protected Methods
	//-------------------------------------------------------------------------
    bool            tokenizeExpression  ( FilterTokenArray & tokens, const cgString & expression );
    bool            preProcessExpression( FilterTokenArray & tokens, const FilterDefineMap * const defines );
    bool            compileExpression   ( FilterInstructionArray & instructions, const FilterTokenArray & tokens, bool optimize );
    
    //-------------------------------------------------------------------------
	// Protected Variables
	//-------------------------------------------------------------------------
    FilterInstructionArray  mInstructions;    // List of instructions to execute during evaluation.
};

#endif // !_CGE_CGFILTEREXPRESSION_H_