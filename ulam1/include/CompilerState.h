/* -*- c++ -*- */

#ifndef COMPILERSTATE_H
#define COMPILERSTATE_H

#include <string>
#include <string.h>
#include <vector>
#include <map>
#include "itype.h"
#include "Tokenizer.h"
#include "NodeBlock.h"
#include "NodeCast.h"
#include "UlamType.h"
#include "Token.h"
#include "ErrorMessageHandler.h"
#include "StringPool.h"
#include "SymbolTable.h"
#include "CallStack.h"
#include "UlamAtom.h"

namespace MFM{

  struct less_than_key
  {
    inline bool operator() (const UlamKeyTypeSignature key1, const UlamKeyTypeSignature key2)
    { 
      if(strcmp(key1.m_typeName, key2.m_typeName) < 0) return true;
      if(strcmp(key1.m_typeName, key2.m_typeName) > 0) return false;  
      if(key1.m_bits < key2.m_bits) return true;
      if(key1.m_bits > key2.m_bits) return false;
      if(key1.m_arraySize < key2.m_arraySize) return true;
      if(key1.m_arraySize > key2.m_arraySize) return false;
      return false; 
    }
  };

  class Symbol;         //forward
  class NodeBlockClass; //forward

  struct CompilerState
  {
    // tokenizer ptr replace by StringPool, service for deferencing strings 
    // e.g., Token identifiers that are variables, path names read by SS, etc.)
    StringPool m_pool;

    NodeBlock * m_currentBlock;     //replaces m_stackOfBlocks
    NodeBlockClass * m_classBlock;  //holds ST with function defs

    s32 m_currentFunctionBlockDeclSize;   //used to calc framestack size for function def
    s32 m_currentFunctionBlockMaxDepth;   //framestack for function def saved in NodeBlockFunction

    CallStack m_funcCallStack;    //local variables and arguments
    UlamAtom m_selectedAtom;      //storage for data member (static/global) variables
    CallStack m_nodeEvalStack;    //for node eval return values, 
                                  //uses evalNodeProlog/Epilog; EVALRETURN storage

    ErrorMessageHandler m_err;

    std::vector<UlamType *> m_indexToUlamType;   //ulamtype ptr by index
    std::map<UlamKeyTypeSignature, UTI, less_than_key> m_definedUlamTypes;   //key -> index of ulamtype (UTI) 

    CompilerState();
    ~CompilerState();

    void clearAllDefinedUlamTypes();

    UlamType * makeUlamType(Token typeTok, u32 bitsize, u32 arraysize);
    UTI makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype);
    bool isDefined(UlamKeyTypeSignature key, UTI& foundUTI);
    UlamType * createUlamType(UlamKeyTypeSignature key, UTI uti, ULAMTYPE utype);

    UlamType * getUlamTypeByIndex(UTI uti);
    const char * getUlamTypeNameByIndex(UTI uti);
    UTI getUlamTypeIndex(UlamType * ut);

    ULAMTYPE getBaseTypeFromToken(Token tok);
    bool getUlamTypeByTypedefName(const char * name, UlamType* & rtnType);

    UlamType * getUlamTypeAsScalar(UlamType * utArg); //turns array into its single element type

    /** return true and the Symbol pointer in 2nd arg if found;
	search SymbolTables LIFO order; o.w. return false
    */
    bool alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr);
    void addSymbolToCurrentScope(Symbol * symptr); //ownership goes to the block


    /** helper methods for error messaging, uses string pool */
    const std::string getTokenLocationAsString(Token * tok);
    const std::string getFullLocationAsString(Locator& loc);
    const std::string getPathFromLocator(Locator& loc);

    /** helper method, uses string pool */
    const std::string getDataAsString(Token * tok);
    const std::string getTokenAsATypeName(Token tok);
  };
  
}

#endif //end COMPILERSTATE_H
