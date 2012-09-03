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
// Name : cgFilterExpression.cppp                                            //
//                                                                           //
// Desc : A utility class that allows for complex (string based) boolean     //
//        expressions to be supplied that will be compiled/interpreted for   //
//        use in evaluations of set/unset bits in a supplied 64 bit value.   //
//                                                                           //
//---------------------------------------------------------------------------//
//      Copyright (c) 1997 - 2010 Game Institute. All Rights Reserved.       //
//---------------------------------------------------------------------------//

//-----------------------------------------------------------------------------
// Precompiled Header
//-----------------------------------------------------------------------------
#include <cgPrecompiled.h>

//-----------------------------------------------------------------------------
// cgFilterExpression Module Includes
//-----------------------------------------------------------------------------
#include <System/cgFilterExpression.h>

//-----------------------------------------------------------------------------
//  Name : cgFilterExpression () (Constructor)
/// <summary>
/// cgFilterExpression Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgFilterExpression::cgFilterExpression()
{
}

//-----------------------------------------------------------------------------
//  Name : cgFilterExpression () (Constructor)
/// <summary>
/// cgFilterExpression Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgFilterExpression::cgFilterExpression( const cgFilterExpression * pInit )
{
    mInstructions = pInit->mInstructions;
}

//-----------------------------------------------------------------------------
//  Name : cgFilterExpression () (Constructor)
/// <summary>
/// cgFilterExpression Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgFilterExpression::cgFilterExpression( const cgString & strExpression )
{
    compileExpression( strExpression );
}

//-----------------------------------------------------------------------------
//  Name : cgFilterExpression () (Constructor)
/// <summary>
/// cgFilterExpression Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
cgFilterExpression::cgFilterExpression( const cgString & strExpression, const IdentifierArray & Defines )
{
    compileExpression( strExpression, Defines );
}

//-----------------------------------------------------------------------------
//  Name : cgFilterExpression () (Destructor)
/// <summary>
/// cgImage Class Destructor
/// </summary>
//-----------------------------------------------------------------------------
cgFilterExpression::~cgFilterExpression()
{
    // Clean up allocated memory
    dispose( false );
}

//-----------------------------------------------------------------------------
//  Name : dispose () (Virtual)
/// <summary>
/// Release any memory, references or resources allocated by this object.
/// </summary>
/// <copydetails cref="cgScriptInterop::DisposableScriptObject::dispose()" />
//-----------------------------------------------------------------------------
void cgFilterExpression::dispose( bool bDisposeBase )
{
    // Clear out compiled instructions.
    mInstructions.clear();

    // Dispose of base class
    if ( bDisposeBase == true )
        DisposableScriptObject::dispose( true );
}

//-----------------------------------------------------------------------------
//  Name : isCompiled () 
/// <summary>
/// Has an expression been successfully compiled?
/// </summary>
//-----------------------------------------------------------------------------
bool cgFilterExpression::isCompiled( ) const
{
    return (!mInstructions.empty());
}

//-----------------------------------------------------------------------------
//  Name : compileExpression () 
/// <summary>
/// Compile the specified filter expression into its component instructions
/// such that it can be executed during evaluation.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFilterExpression::compileExpression( const cgString & strExpression )
{
    // Clear out the existing instruction set.
    mInstructions.clear();

    // Tokenize, pre-process and compile the new expression
    FilterTokenArray aTokens;
    if ( !tokenizeExpression( aTokens, strExpression ) ||
         !preProcessExpression( aTokens, CG_NULL ) ||
         !compileExpression( mInstructions, aTokens, true ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to compile filter expression '%s'. See previous errors for more information.\n"), strExpression.c_str() );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : compileExpression () 
/// <summary>
/// Compile the specified filter expression into its component instructions
/// such that it can be executed during evaluation. A dictionary of string
/// based 'defines' that describe the value to be compared can also be
/// supplied.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFilterExpression::compileExpression( const cgString & strExpression, const IdentifierArray & Defines )
{
    // Clear out the existing instruction set.
    mInstructions.clear();

    // Build a map of all defined identifiers.
    FilterDefineMap DefineLUT;
    for ( size_t i = 0; i < Defines.size(); ++i )
        DefineLUT[Defines[i].name] = Defines[i].value;

    // Tokenize, pre-process and compile the new expression
    FilterTokenArray aTokens;
    if ( !tokenizeExpression( aTokens, strExpression ) ||
         !preProcessExpression( aTokens, &DefineLUT ) ||
         !compileExpression( mInstructions, aTokens, true ) )
    {
        cgAppLog::write( cgAppLog::Error, _T("Failed to compile filter expression '%s'. See previous errors for more information.\n"), strExpression.c_str() );
        return false;
    
    } // End if failed

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : evaluate () 
/// <summary>
/// Execute the compiled expression in order to evaluate the bits of the 
/// specified value.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFilterExpression::evaluate( cgUInt64 Value )
{
    static int Result;
    static int TempRegisters[256];
    static const FilterInstruction * pc, * pce;
    static int * r;   

    // Automatically matches if there is nothing to compare.
    if ( mInstructions.empty() )
        return true;

    // Initialize first result register.
    TempRegisters[0] = 1;

    // Process instructions.
    r = TempRegisters;
    pc = &mInstructions.front();
    pce = pc + mInstructions.size();
    while ( pc != pce )
    {
        // Note: This is just about the fastest ordering that I have found
        // so far, faster by a full 33% than the worst case ordering encountered.
        switch ( pc->opCode )
        {
            case OpRAnd:
                *r &= Result;
                break;

            case OpNAnd:
                *r &= !((Value & pc->bits) == pc->bits);
                break;

            case OpRNAnd:
                *r &= !Result;
                break;

            case OpOr:
                *r |= ((Value & pc->bits) != 0);
                break;

            case OpROr:
                *r |= Result;
                break;

            case OpNOr:
                *r |= ((Value & pc->bits) == 0);
                break;
                
            case OpRNOr:
                *r |= !Result;
                break;

            case OpSet:
                *r = ((Value & pc->bits) == pc->bits);
                break;

            case OpRSet:
                *r = Result;
                break;

            case OpNSet:
                *r = !((Value & pc->bits) == pc->bits);
                break;

            case OpRNSet:
                *r = !Result;
                break;

            case OpAnd:
                *r &= ((Value & pc->bits) == pc->bits);
                break;

            case OpAndJz:
                *r &= ((Value & pc->bits) == pc->bits);
                pc += pc->jumpCount * (!*r);
                break;

            case OpRAndJz:
                *r &= Result;
                pc += pc->jumpCount * (!*r);
                break;

            case OpNAndJz:
                *r &= !((Value & pc->bits) == pc->bits);
                pc += pc->jumpCount * (!*r);
                break;

            case OpRNAndJz:
                *r &= !Result;
                pc += pc->jumpCount * (!*r);
                break;

            case OpSetJz:
                *r = ((Value & pc->bits) == pc->bits);
                pc += pc->jumpCount * (!*r);
                break;

            case OpRSetJz:
                *r = Result;
                pc += pc->jumpCount * (!*r);
                break;

            case OpNSetJz:
                *r = !((Value & pc->bits) == pc->bits);
                pc += pc->jumpCount * (!*r);
                break;

            case OpRNSetJz:
                *r = !Result;
                pc += pc->jumpCount * (!*r);
                break;
            
            default:
                
                // Nested switch statement helps improve performance
                switch ( pc->opCode )
                {
                    case OpPush:
                        // Backup current result so far and initialize current result 
                        // register for a new series of operations.
                        *++r = 1;
                        break;

                    case OpPop:

                        // Copy the computed result at this depth into the temporary result
                        // variable and then restore the prior result that was previously being built.
                        Result = *r--;
                        break;

                }
                break;

        } // End Switch opCode

        // Move to next instruction
        ++pc;

    } // Next Instruction

    // Return the final result.
    return (*r != 0);
}

//-----------------------------------------------------------------------------
//  Name : tokenizeExpression () (Protected)
/// <summary>
/// Parse the supplied expression string and break it down into its component
/// tokens that are more easily processed.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFilterExpression::tokenizeExpression( FilterTokenArray & Tokens, const cgString & strExpression )
{
    size_t nLength = strExpression.size();
    for ( size_t i = 0; i < nLength; ++i )
    {
        cgTChar cCurrent = strExpression.at(i);

        switch ( cCurrent )
        {
            case _T('&'):
                // Skip multiple '&' characters (i.e. application uses '&&' )
                if ( !Tokens.empty() && Tokens.back().type == TokenAnd )
                    break;
                Tokens.push_back( FilterToken( TokenAnd, i ) );
                break;

            case _T('|'):
                // Skip multiple '|' characters (i.e. application uses '||' )
                if ( !Tokens.empty() && Tokens.back().type == TokenOr )
                    break;
                Tokens.push_back( FilterToken( TokenOr, i ) );
                break;

            case _T('!'):
                Tokens.push_back( FilterToken( TokenNot, i ) );
                break;

            case _T('('):
                Tokens.push_back( FilterToken( TokenOpenParen, i ) );
                break;

            case _T(')'):
                Tokens.push_back( FilterToken( TokenCloseParen, i ) );
                break;

            case _T(' '):
            case _T('\r'):
            case _T('\n'):
            case _T('\t'):
                if ( Tokens.empty() || Tokens.back().type != TokenWhitespace )
                    Tokens.push_back( FilterToken( TokenWhitespace, i ) );
                break;

            default:
                // Add an identifier token if we are not already building one.
                if ( Tokens.empty() || Tokens.back().type != TokenIdentifier )
                    Tokens.push_back( FilterToken( TokenIdentifier, i ) );
                Tokens.back().valueName.append( 1, cCurrent );
                break;
        
        } // End switch cCurrent

    } // Next Character

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : preProcessExpression () (Protected)
/// <summary>
/// Take a pass over the tokenized expression and perform validation as well
/// as replacing any identifiers with their defined values.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFilterExpression::preProcessExpression( FilterTokenArray & Tokens, const FilterDefineMap * const pDefines )
{
    // Find first non whitespace token.
    size_t nFirstToken = 0;
    for ( ; nFirstToken < Tokens.size(); ++nFirstToken )
    {
        // Not whitespace?
        if ( Tokens[nFirstToken].type != TokenWhitespace )
            break;
    
    } // Next token

    // First token should not be '&', '|' or ')'
    if ( Tokens[nFirstToken].type == TokenAnd )
    {
        cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token '&' at offset %i. Valid opening token must be an identifier, an opening parenthesis or a logical not '!'.\n"), Tokens[0].offset );
        return false;
    
    } // End if invalid token
    else if ( Tokens[nFirstToken].type == TokenOr )
    {
        cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token '|' at offset %i. Valid opening token must be an identifier, an opening parenthesis or a logical not '!'.\n"), Tokens[0].offset );
        return false;
    
    } // End if invalid token
    else if ( Tokens[nFirstToken].type == TokenCloseParen )
    {
        cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token ')' at offset %i. Valid opening token must be an identifier, an opening parenthesis or a logical not '!'.\n"), Tokens[0].offset );
        return false;
    
    } // End if invalid token

    // Process tokens.
    cgInt nDepth = 0;
    FilterTokenType PrevType = TokenOpenParen; // We use TokenOpenParen since it is a valid "previous" token for all starting tokens.
    for ( size_t i = nFirstToken; i < Tokens.size(); ++i )
    {
        FilterToken & Token = Tokens[i];
        switch ( Token.type )
        {
            case TokenWhitespace:
                // Ignore ALL whitespace.
                continue;

            case TokenIdentifier:
                // Previous token for identifiers must be an opening parenthesis, or a logical operator.
                if ( PrevType == TokenCloseParen || PrevType == TokenIdentifier )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected identifier '%s' at offset %i.\n"), Token.valueName.c_str(), Tokens[i].offset );
                    return false;
                
                } // End if invalid

                // Resolve identifier if requested
                if ( pDefines )
                {
                    FilterDefineMap::const_iterator itDefine = pDefines->find( Token.valueName );
                    if ( itDefine != pDefines->end() )
                    {
                        Token.valueData = itDefine->second;
                    
                    } // End if match
                    else
                    {
                        cgAppLog::write( cgAppLog::Error, _T("'%s' : undeclared identifier at offset %i.\n"), Token.valueName.c_str(), Tokens[i].offset );
                        return false;
                    
                    } // End if no match

                } // End if resolve
                break;

            case TokenOpenParen:
                // Previous token for opening parenthesis must be another opening parenthesis, or a logical operator.
                if ( PrevType == TokenCloseParen || PrevType == TokenIdentifier )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token '(' at offset %i.\n"), Tokens[i].offset );
                    return false;
                
                } // End if invalid

                // Increase current depth.
                ++nDepth;
                break;

            case TokenCloseParen:
                // Previous token must be either an identifier or another closing parenthesis.
                // There must also have been a prior opening parenthesis to match against.
                if ( (PrevType != TokenCloseParen && PrevType != TokenIdentifier) || nDepth == 0 )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token ')' at offset %i.\n"), Tokens[i].offset );
                    return false;
                
                } // End if invalid

                // Decrease current depth.
                --nDepth;
                break;

            case TokenAnd:
                // Previous token must be an identifier or a closing parenthesis
                if ( PrevType != TokenIdentifier && PrevType != TokenCloseParen )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token '&' at offset %i.\n"), Tokens[i].offset );
                    return false;
                
                } // End if invalid
                break;

            case TokenOr:
                // Previous token must be an identifier or a closing parenthesis
                if ( PrevType != TokenIdentifier && PrevType != TokenCloseParen )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token '|' at offset %i.\n"), Tokens[i].offset );
                    return false;
                
                } // End if invalid
                break;

            case TokenNot:
                // Previous token must be a logical operator or an opening parenthesis
                if ( PrevType != TokenAnd && PrevType != TokenOr && PrevType != TokenOpenParen )
                {
                    cgAppLog::write( cgAppLog::Error, _T("Syntax error: unexpected token '!' at offset %i.\n"), Tokens[i].offset );
                    return false;
                
                } // End if invalid
                break;

        } // End switch Token.type

        // Record previous token for validation of following token.
        PrevType = Token.type;

    } // Next Token

    // Mismatched parentheses?
    if ( nDepth > 0 )
    {
        cgAppLog::write( cgAppLog::Error, _T("Syntax error: missing ')' before end of expression.\n") );
        return false;
    
    } // End if missing ')'

    // Success!
    return true;
}

//-----------------------------------------------------------------------------
//  Name : compileExpression () (Protected)
/// <summary>
/// Generate evaluation VM instructions ("byte code") from the expression
/// tokens.
/// </summary>
//-----------------------------------------------------------------------------
bool cgFilterExpression::compileExpression( FilterInstructionArray & Instructions, const FilterTokenArray & Tokens, bool bOptimize )
{
    std::stack<FilterOpCodes> OperatorStack;
    std::stack<FilterInstructionArray*> InstructionStack;
    std::stack<size_t> LastInstructionStack;
    std::stack<bool> TermGroupStack;

    // Prepare stacks
    OperatorStack.push( OpSet );
    TermGroupStack.push( false );
    InstructionStack.push( new FilterInstructionArray() );
    LastInstructionStack.push(0);

    // ToDo: Move optimization of AND->AND and SET->AND into the optimizing pass
    // to reduce complexity of token processing?

    // ToDo: Stack must not get larger than 256 (maximum size of temporary result
    // stack within the evaluation function)

    // Process tokens
    for ( size_t i = 0; i < Tokens.size(); ++i )
    {
        FilterInstructionArray * pInstructions = InstructionStack.top();
        const FilterToken & Token = Tokens[i];
        switch ( Token.type )
        {
            case TokenWhitespace:

                // Ignore ALL whitespace.
                continue;

            case TokenOpenParen:

                // Transform explicit value opreation into a result operation
                switch ( OperatorStack.top() )
                {
                    case OpSet:
                        OperatorStack.top() = OpRSet;
                        break;
                    case OpNSet:
                        OperatorStack.top() = OpRNSet;
                        break;
                    case OpAnd:
                        OperatorStack.top() = OpRAnd;
                        break;
                    case OpNAnd:
                        OperatorStack.top() = OpRNAnd;
                        break;
                    case OpOr:
                        OperatorStack.top() = OpROr;
                        break;
                    case OpNOr:
                        OperatorStack.top() = OpRNOr;
                        break;
                
                } // End switch operator

                // Create a new list of instructions for this new depth.
                InstructionStack.push( new FilterInstructionArray() );
                pInstructions = InstructionStack.top();

                // Push temporary registers and create a new stack entry.
                pInstructions->push_back( FilterInstruction( OpPush ) );

                // Prepare for a new set of operations (within parentheses)
                OperatorStack.push( OpSet );
                TermGroupStack.push( false );
                LastInstructionStack.push(0);
                break;

            case TokenCloseParen:
            {
                // We may need to do this twice if we were previously grouping terms.
                size_t nIterations = (TermGroupStack.top()) ? 2 : 1;
                for ( size_t j = 0; j < nIterations; ++j )
                {
                    // Pop temporary data.
                    pInstructions->push_back( FilterInstruction( OpPop ) );
                    OperatorStack.pop();
                    TermGroupStack.pop();
                    LastInstructionStack.pop();
                    
                    // Perform the operation based on the result generated.
                    pInstructions->push_back( FilterInstruction( OperatorStack.top() ) );
                    
                    // The generated instructions at this depth are inserted into 
                    // the parent instruction list.
                    InstructionStack.pop();
                    FilterInstructionArray * pDst = InstructionStack.top();
                    LastInstructionStack.top() = pDst->size();
                    pDst->insert( pDst->end(), pInstructions->begin(), pInstructions->end() );
                    delete pInstructions;

                    // Parent list becomes the current list
                    pInstructions = InstructionStack.top();
                
                } // Next iteration
                break;
            
            } // End case TokenCloseParen
            case TokenAnd:
            {
                // If the previous operation was an 'Or', we /always/ need to group
                // the result of the and-ed terms by simulating parentheses (operator 
                // prescedence rules).
                FilterInstruction & PrevInstruction = pInstructions->back();
                if ( PrevInstruction.opCode == OpOr || PrevInstruction.opCode == OpNOr ||
                     PrevInstruction.opCode == OpROr || PrevInstruction.opCode == OpRNOr )
                {
                    // Create a new list of instructions for this new depth.
                    InstructionStack.push( new FilterInstructionArray() );
                    FilterInstructionArray * pDst = InstructionStack.top();

                    // We need to insert a 'push' just prior to the previous instruction(s).
                    pDst->push_back( FilterInstruction( OpPush ) );

                    // Convert previous instruction to an appropriate 'set'.
                    switch ( PrevInstruction.opCode )
                    {
                        case OpOr:
                            PrevInstruction.opCode = OpSet;
                            OperatorStack.top() = OpROr;
                            break;
                        case OpNOr:
                            PrevInstruction.opCode = OpNSet;
                            OperatorStack.top() = OpROr;
                            break;
                        case OpROr:
                            PrevInstruction.opCode = OpRSet;
                            break;
                        case OpRNOr:
                            PrevInstruction.opCode = OpRNSet;
                            OperatorStack.top() = OpROr;
                            break;

                    } // End switch opCode

                    // Move all prior instructions from the start of the last instruction set
                    // (could be a full push->pop set) onto the new list.
                    pDst->insert( pDst->begin() + 1, pInstructions->begin() + LastInstructionStack.top(), pInstructions->end() );

                    // Remove from prior instruction list.
                    pInstructions->resize( LastInstructionStack.top() );
                    
                    // Prepare for a new set of operations (within parentheses)
                    OperatorStack.push( OpAnd );
                    TermGroupStack.push( true );
                    LastInstructionStack.push(0);
                    break;
                    
                } // End if group terms
                else
                {
                    OperatorStack.top() = OpAnd;
                    break;

                } // End if !group terms
            
            } // End case TokenAnd
            case TokenOr:

                // If we were previously grouping terms, close it.
                if ( TermGroupStack.top() )
                {
                    // Pop temporary data.
                    pInstructions->push_back( FilterInstruction( OpPop ) );
                    OperatorStack.pop();
                    TermGroupStack.pop();
                    LastInstructionStack.pop();
                    
                    // Perform the operation based on the result generated.
                    pInstructions->push_back( FilterInstruction( OperatorStack.top() ) );
                    
                    // The generated instructions at this depth are inserted into 
                    // the parent instruction list.
                    InstructionStack.pop();
                    FilterInstructionArray * pDst = InstructionStack.top();
                    LastInstructionStack.top() = pDst->size();
                    pDst->insert( pDst->end(), pInstructions->begin(), pInstructions->end() );
                    delete pInstructions;
                
                } // End if grouping terms
                
                OperatorStack.top() = OpOr;
                break;

            case TokenNot:
                
                // Transform operation into its inverted (not) version.
                switch ( OperatorStack.top() )
                {
                    case OpSet:
                        OperatorStack.top() = OpNSet;
                        break;
                    case OpAnd:
                        OperatorStack.top() = OpNAnd;
                        break;
                    case OpOr:
                        OperatorStack.top() = OpNOr;
                        break;
                
                } // End switch operator
                break;

            case TokenIdentifier:
            {
                // Perform the operation.
                bool bNewInstruction = true;
                if ( bOptimize && !pInstructions->empty() )
                {
                    FilterInstruction & PrevInstruction = pInstructions->back();
                    if ( OperatorStack.top() == OpAnd && PrevInstruction.opCode == OpAnd )
                    {
                        //std::cout << "Collapsing AND->AND instructions\n";
                        PrevInstruction.bits |= Token.valueData;
                        bNewInstruction = false;
                    
                    } // End if And->And
                    else if ( OperatorStack.top() == OpAnd && PrevInstruction.opCode == OpSet )
                    {
                        //std::cout << "Collapsing SET->AND instruction\n";
                        PrevInstruction.opCode = OpAnd;
                        PrevInstruction.bits |= Token.valueData;
                        bNewInstruction = false;
                    
                    } // End if Set->And
                
                } // End if can optimize
                
                // Do we need a new instruction (not able to collapse)?
                if ( bNewInstruction )
                {
                    LastInstructionStack.top() = pInstructions->size();
                    pInstructions->push_back( FilterInstruction( OperatorStack.top(), Token.valueData ) );
                
                } // End if new instruction
                break;

            } // End case TokenIdentifier

        } // End switch Token.type

    } // Next Token

    // If we were previously grouping terms, close it.
    if ( TermGroupStack.top() )
    {
        FilterInstructionArray * pInstructions = InstructionStack.top();
        pInstructions->push_back( FilterInstruction( OpPop ) );
        OperatorStack.pop();

        // Perform the operation based on the result generated.
        pInstructions->push_back( FilterInstruction( OperatorStack.top() ) );

        // The generated instructions at this depth are inserted into 
        // the parent instruction list.
        InstructionStack.pop();
        FilterInstructionArray * pDst = InstructionStack.top();
        pDst->insert( pDst->end(), pInstructions->begin(), pInstructions->end() );
        delete pInstructions;
        
    } // End if grouping terms

    // Copy out the final instructions.
    FilterInstructionArray * pInstructions = InstructionStack.top();
    Instructions.insert( Instructions.end(), pInstructions->begin(), pInstructions->end() );
    delete pInstructions;

    // Perform some optimizations?
    if ( bOptimize )
    {
        // Generate early out (JMP) information
        for ( size_t i = 0; i < Instructions.size(); ++i )
        {
            FilterInstruction & Instruction = Instructions[i];
            if ( Instruction.opCode == OpAnd || Instruction.opCode == OpRAnd ||
                 Instruction.opCode == OpNAnd || Instruction.opCode == OpRNAnd ||
                 Instruction.opCode == OpSet || Instruction.opCode == OpRSet ||
                 Instruction.opCode == OpNSet || Instruction.opCode == OpRNSet )
            {
                // Find the next available instruction to jump to if the test fails.
                size_t nLastPushInstruction = 0;
                for ( size_t j = i + 1; j < Instructions.size(); ++j )
                {
                    const FilterInstruction & JumpInstruction = Instructions[j];
                    if ( JumpInstruction.opCode == OpOr || JumpInstruction.opCode == OpNOr ||
                         JumpInstruction.opCode == OpPop )
                    {
                        Instruction.jumpCount = j - i;
                        break;
                    
                    } // End if 'Or' or 'Pop' variant
                    else if ( JumpInstruction.opCode == OpROr || JumpInstruction.opCode == OpRNOr )
                    {
                        Instruction.jumpCount = nLastPushInstruction - i;
                        break;

                    } // End if 'ROr' variant
                    else if ( JumpInstruction.opCode == OpPush )
                    {
                        nLastPushInstruction = j++;

                        // Find associated pop
                        int nDepth = 1;
                        while ( j < Instructions.size() )
                        {
                            if ( Instructions[j].opCode == OpPush )
                                nDepth++;
                            else if ( Instructions[j].opCode == OpPop )
                                nDepth--;

                            if ( nDepth == 0 )
                                break;
                            
                            ++j;

                        } // Next search iteration

                    } // End if 'Push' variant

                } // Next instruction

                // Jump out completely if we reached the end.
                if ( !Instruction.jumpCount )
                    Instruction.jumpCount = Instructions.size() - i;

                // Transform to a 'jump if zero' instruction where possible.
                if ( Instruction.jumpCount )
                {
                    switch ( Instruction.opCode )
                    {
                        case OpSet:
                            Instruction.opCode = OpSetJz;
                            break;
                        case OpAnd:
                            Instruction.opCode = OpAndJz;
                            break;
                        case OpNSet:
                            Instruction.opCode = OpNSetJz;
                            break;
                        case OpNAnd:
                            Instruction.opCode = OpNAndJz;
                            break;
                        case OpRSet:
                            Instruction.opCode = OpRSetJz;
                            break;
                        case OpRAnd:
                            Instruction.opCode = OpRAndJz;
                            break;
                        case OpRNSet:
                            Instruction.opCode = OpRNSetJz;
                            break;
                        case OpRNAnd:
                            Instruction.opCode = OpRNAndJz;
                            break;
                    
                    } // End switch operator

                    // Subtract 1 from the jump count to avoid
                    // the need to do it in the comparison function
                    // (program counter is always incremented)
                    --Instruction.jumpCount;

                } // End if will jump

            } // End if 'And' variant

        } // Next instruction

    } // End if optimize

    // Success!
    return true;
}