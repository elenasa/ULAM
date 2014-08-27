#include <stdio.h>
#include <iostream>
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "SymbolTable.h"
#include "SymbolTypedef.h"
#include "SymbolVariable.h"
#include "UlamTypeAtom.h"
#include "UlamTypeBool.h"
#include "UlamTypeFloat.h"
#include "UlamTypeInt.h"
#include "UlamTypeNav.h"
#include "UlamTypeVoid.h"


namespace MFM {

#define _DEBUG_OUTPUT 
#ifdef _DEBUG_OUTPUT
  static const bool debugOn = true;
#else
  static const bool debugOn = false;
#endif

  static const char * m_indentedSpaceLevel("  ");


  CompilerState::CompilerState(): m_currentBlock(NULL), m_classBlock(NULL), m_useMemberBlock(false), m_currentMemberClassBlock(NULL), m_currentFunctionBlockDeclSize(0), m_currentFunctionBlockMaxDepth(0)
  {
    m_err.init(this, debugOn, NULL);
  }


  CompilerState::~CompilerState()
  {
    clearAllDefinedUlamTypes();
  }


  void CompilerState::clearAllDefinedUlamTypes()
  {
    for(u32 i = 0; i < m_indexToUlamType.size(); i++)
      {
	UlamType * ut = m_indexToUlamType.at(i);
	delete ut;
      }
    m_indexToUlamType.clear();
    m_definedUlamTypes.clear();
    m_currentFunctionReturnNodes.clear();
  }


  //convenience method (refactors code originally from installSymbol)
  //if exists, just returns it, o.w. makes it;
  // trick to know the base ULAMTYPE
  UlamType * CompilerState::makeUlamType(Token typeTok, u32 bitsize, u32 arraysize)
  {
    //type names begin with capital letter..and the rest can be either
    u32 typeNameId  = getTokenAsATypeNameId(typeTok); //Foo, Int, etc

    // is this name already a typedef for a complex type?
    ULAMTYPE bUT = getBaseTypeFromToken(typeTok);

    bitsize = ((bitsize == 0 && bUT != Class) ? (bUT == Bool ? BITSPERBOOL : 32) : bitsize); //temporary!!!
    UlamKeyTypeSignature key(typeNameId,bitsize,arraysize);
    UTI uti = 0;
    UlamType * ut;

    if(isDefined(key,uti))
      {
	ut = getUlamTypeByIndex(uti);
      }
    else
      {
	//no key, make new type, how to know baseUT? bitsize?
	uti = makeUlamType(key,bUT);    
	ut = getUlamTypeByIndex(uti);
      }
    return ut;
  }


  UTI CompilerState::makeUlamType(UlamKeyTypeSignature key, ULAMTYPE utype)
  {
    UTI uti;
    if(!isDefined(key, uti))
      {
	uti = m_indexToUlamType.size();  //custom index based on key
	UlamType * ut = createUlamType(key, uti, utype);
	m_indexToUlamType.push_back(ut);  //vector owns ut
	m_definedUlamTypes.insert(std::pair<UlamKeyTypeSignature,UTI>(key,uti));
	assert(isDefined(key, uti));
      }
    return uti;
  }


  bool CompilerState::isDefined(UlamKeyTypeSignature key, UTI& foundUTI)
  {
    bool rtnBool= false;

    std::map<UlamKeyTypeSignature, UTI, less_than_key>::iterator it = m_definedUlamTypes.begin();

    while(it != m_definedUlamTypes.end())
      {
	if(key == it->first)
	  {
	    foundUTI = it->second;  
	    rtnBool = true;
	    break;
	  }
	it++;
      }

    return rtnBool;
  }


  UlamType * CompilerState::createUlamType(UlamKeyTypeSignature key, UTI uti, ULAMTYPE utype)
  {
    UlamType * ut = NULL;

    switch(utype)
      {
      case Nav:
	ut = new UlamTypeNav(key, uti);      
	break;
      case Void:
	ut = new UlamTypeVoid(key, uti);      
	break;
      case Int:
	ut = new UlamTypeInt(key, uti);      
	break;
      case Float:
	ut = new UlamTypeFloat(key, uti);      
	break;
      case Bool:
	ut = new UlamTypeBool(key, uti);      
	break;
      case Class:
	ut = new UlamTypeClass(key, uti);      
	break;
      case Atom:
	ut = new UlamTypeAtom(key, uti);      
	break;
      default:
	{
	  std::ostringstream msg;
	  msg << "Undefined ULAMTYPE base type <" << utype << ">" ;
	  MSG2("",msg.str().c_str(),DEBUG);
	  assert(0);
	}
      };

    return ut;
  }


  UlamType * CompilerState::getUlamTypeByIndex(UTI typidx)
  {
    if(typidx >= m_indexToUlamType.size())
      {
	std::ostringstream msg;
	msg << "Undefined UTI <" << typidx << "> Max is: "
	    << m_indexToUlamType.size() << ", returning Nav INSTEAD";
	MSG2("", msg.str().c_str(),DEBUG);
	typidx = 0;
      }
    return m_indexToUlamType[typidx];
  }


  const std::string CompilerState::getUlamTypeNameByIndex(UTI uti)
  {
    return m_indexToUlamType[uti]->getUlamTypeNameBrief(this);
  }


  UTI CompilerState::getUlamTypeIndex(UlamType * ut)
  {
    UlamKeyTypeSignature key = ut->getUlamKeyTypeSignature();
    UTI rtnUTI = 0;         //Nav
    isDefined(key,rtnUTI);  //updates rtnUTI if found
    return rtnUTI;
  }


  ULAMTYPE CompilerState::getBaseTypeFromToken(Token tok)
  {
    std::string typeName  = getTokenAsATypeName(tok); //Foo, Int, etc

    // is this name already a typedef for a complex type?
    UlamType * ut = NULL;
    ULAMTYPE bUT = Nav;
    if(getUlamTypeByTypedefName(typeName.c_str(), ut))
      {
	bUT = ut->getUlamTypeEnum();
      }
    else
      {
	if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    //no way to get the bUT, except to assume typeName is one of them?
	    bUT = UlamType::getEnumFromUlamTypeString(typeName.c_str()); //could be Element, etc.;
	  }
	else
	  {
	    SymbolClass * csym = NULL;
	    if(alreadyDefinedSymbolClass(tok.m_dataindex, csym))
	      {
		UlamType * ut = csym->getUlamType();
		bUT = ut->getUlamTypeEnum();
	      }
	  }
      }
    return bUT;
  }


  UlamType * CompilerState::getUlamTypeFromToken(Token tok)
  {
    UlamType * ut = NULL;
    std::string typeName  = getTokenAsATypeName(tok); //Foo, Int, etc

    // is this name already a typedef for a complex type?
    if(!getUlamTypeByTypedefName(typeName.c_str(), ut))
      {
	if(Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD)
	  {
	    ULAMTYPE bUT = UlamType::getEnumFromUlamTypeString(typeName.c_str()); //could Int, Bool, Void, Float;
	    ut = getUlamTypeByIndex((UTI) bUT);
	  }
	else
	  {
	    //check for a Class type, or make one if doesn't exist yet, while parsing.
	    SymbolClass * csym = NULL;
	    if(alreadyDefinedSymbolClass(tok.m_dataindex, csym))
	      {
		ut = csym->getUlamType();
	      }
	  }
      }
    return ut;
  }


  bool CompilerState::getUlamTypeByTypedefName(const char * name, UlamType* & rtnType)
  {
    bool rtnBool = false;
    Symbol * asymptr = NULL;
    std::string nameStr(name);
    u32 idx = m_pool.getIndexForDataString(nameStr);

    //searched back through all block's ST for idx
    if(alreadyDefinedSymbol(idx, asymptr))
      {
	if(asymptr->isTypedef())
	  {
	    rtnType = asymptr->getUlamType();
	    rtnBool = true; 
	  }
      }

    return rtnBool;
  }


  UlamType * CompilerState::getUlamTypeAsScalar(UlamType * utArg)
  {
    if(utArg->isScalar())
      return utArg;

    // for typedef array, the scalar is the primitive type
    ULAMTYPE bUT = utArg->getUlamTypeEnum();
    UlamKeyTypeSignature keyOfArg = utArg->getUlamKeyTypeSignature();

    u32 bitsize = keyOfArg.getUlamKeyTypeSignatureBitSize();
    UlamKeyTypeSignature baseKey(keyOfArg.m_typeNameId, bitsize);  //default array size is zero

    UTI buti = makeUlamType(baseKey, bUT);
    return getUlamTypeByIndex(buti);
  }


  bool CompilerState::getUlamTypeByClassToken(Token ctok, UlamType* & rtnType)
  {
    u32 cidx = getTokenAsATypeNameId(ctok);
    return getUlamTypeByClassNameId(cidx, rtnType);
  }


  bool CompilerState::getUlamTypeByClassNameId(u32 idx, UlamType* & rtnType)
  {
    bool rtnBool = false;
    SymbolClass * csymptr = NULL;
    
    if(alreadyDefinedSymbolClass(idx, csymptr) || (addIncompleteClassSymbolToProgramTable(idx, csymptr), true) )
      {
	rtnType = csymptr->getUlamType();
	rtnBool = true;
      }

    assert(rtnBool);  //no way it's false!
    return rtnBool;  
  }


  bool CompilerState::alreadyDefinedSymbolClass(u32 dataindex, SymbolClass * & symptr)
  {
    return m_programDefST.isInTable(dataindex,(Symbol * &) symptr);
  }


  //temporary UlamType which will be updated during type labeling.
  void CompilerState::addIncompleteClassSymbolToProgramTable(u32 dataindex, SymbolClass * & symptr) 
  {
    assert(!alreadyDefinedSymbolClass(dataindex,symptr));

    UlamKeyTypeSignature key(dataindex, 0);
    UTI cuti = makeUlamType(key, Class);
    UlamType * cut = getUlamTypeByIndex(cuti);
    
    // symbol ownership goes to the programDefST; 
    symptr = new SymbolClass(dataindex, cut, NULL);  //NodeBlockClass is NULL for now
    m_programDefST.addToTable(dataindex, symptr);
  }


  bool CompilerState::completeIncompleteClassSymbol(UlamType * incomplete) 
  {
    bool rtnB = false;
    SymbolClass * csym = NULL;
    if(alreadyDefinedSymbolClass(incomplete->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
      {
	UlamType * but = csym->getUlamType();
	ULAMCLASSTYPE bc = but->getUlamClassType();
	assert(bc == UC_ELEMENT || bc == UC_QUARK);
	((UlamTypeClass *) incomplete)->setUlamClassType(bc);
	((UlamTypeClass *) incomplete)->setBitSize(but->getBitSize());
	rtnB = true;
      }
    else
      {
	assert(0);
      }

    return rtnB;
  }


  bool CompilerState::alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;

    // start with the current "top" block and look down the stack
    // until the 'variable id' is found.
    NodeBlock * blockNode = m_currentBlock;

    // substitute another selected class block to search for data member
    if(m_useMemberBlock)
      blockNode = m_currentMemberClassBlock;

    while(!brtn && blockNode)
      {
	brtn = blockNode->isIdInScope(dataindex,symptr);
	blockNode = blockNode->getPreviousBlockPointer();  //traverse the chain
      }

    // data member variables in class block; function symbols are linked to their 
    // function def block; check function data members separately.
    if(!brtn)
      {
	brtn = isFuncIdInClassScope(dataindex, symptr);
      }
 
    return brtn;
  }


  bool CompilerState::isFuncIdInClassScope(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;
    if(m_useMemberBlock)
      {
	if(m_currentMemberClassBlock)
	  brtn = m_currentMemberClassBlock->isFuncIdInScope(dataindex,symptr);
      }
    else
      brtn = m_classBlock->isFuncIdInScope(dataindex,symptr);
   
    return brtn;
  }
  

  bool CompilerState::isIdInClassScope(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;
    if(m_useMemberBlock)
      {
	if(m_currentMemberClassBlock)
	  brtn = m_currentMemberClassBlock->isIdInScope(dataindex,symptr);
      }
    else
      brtn = m_classBlock->isIdInScope(dataindex,symptr);
   
    return brtn;
  }


  //symbol ownership goes to the current block (end of vector)
  void CompilerState::addSymbolToCurrentScope(Symbol * symptr) 
  {
    m_currentBlock->addIdToScope(symptr->getId(), symptr);
  }


  //Token to location as string:
  const std::string CompilerState::getTokenLocationAsString(Token * tok)
  {
    if(!tok)
      return std::string("");

    return getFullLocationAsString(tok->m_locator);
  }


  /** combines path, line, and byte number for error messages */
  const std::string CompilerState::getFullLocationAsString(Locator& loc)
  {
    std::ostringstream ss;
    ss << getPathFromLocator(loc) << ":" << loc.getLineNo() << ":" << loc.getByteNo() << ":";
    return ss.str();
  }


  const std::string CompilerState::getPathFromLocator(Locator& loc)
  {
    return m_pool.getDataAsString(loc.getPathIndex());
  }


  const std::string CompilerState::getDataAsString(Token * tok)
  {
    assert(tok);
    return m_pool.getDataAsString(tok->m_dataindex);
  }


  //does it check for existence?
  const std::string CompilerState::getTokenAsATypeName(Token tok)
  {
    if(Token::isTokenAType(tok))
      {
	if((Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD))
	  {
	    return std::string(Token::getTokenAsString(tok.m_type));
	  }
	
 	return getDataAsString(&tok);
      }

    return "Nav";
  }


  u32 CompilerState::getTokenAsATypeNameId(Token tok)
  {
    std::string nstr = getTokenAsATypeName(tok);
    return m_pool.getIndexForDataString(nstr);
  }


  bool CompilerState::checkFunctionReturnNodeTypes(SymbolFunction * fsym)
  {
    bool rtnBool = true;
    UlamType * it = fsym->getUlamType();

    if(m_currentFunctionReturnNodes.empty()) 
      {
	if(it != getUlamTypeByIndex(Void))
	  {
	    std::ostringstream msg;
	    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return Statement is missing";
	    m_err.buildMessage("", msg.str().c_str(), "MFM::NodeFunctionBlock", "checkAndLabelType", -1, MSG_ERR);
	    return false;
	  }
	return true;  //okay to skip return statement for void function
      }
    

    for(u32 i = 0; i < m_currentFunctionReturnNodes.size(); i++)
      {
	NodeReturnStatement * rNode = m_currentFunctionReturnNodes.at(i);
	UlamType * rType = rNode->getNodeType();
	
	if(rType != it)
	  {
	    rtnBool = false;

	    ULAMTYPE rBUT = rType->getUlamTypeEnum();
	    ULAMTYPE itBUT = it->getUlamTypeEnum();
	    if(rBUT != itBUT)
	      {
		std::ostringstream msg;
		msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's <" << it->getUlamTypeName(this).c_str() << "> base type: <" << UlamType::getUlamTypeEnumAsString(itBUT) << "> does not match resulting type's <" << rType->getUlamTypeName(this).c_str() << "> base type: <" << UlamType::getUlamTypeEnumAsString(rBUT) << ">";
		m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", -1, MSG_ERR);

	      }
	    else
	      {
		if(rType->getArraySize() != it->getArraySize())
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's <" << it->getUlamTypeName(this).c_str() << "> array size: <" << it->getArraySize() << "> does not match resulting type's <" << rType->getUlamTypeName(this).c_str() << "> array size: <" << rType->getArraySize() << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", -1, MSG_ERR);
		  }

		if(rType->getBitSize() != it->getBitSize())
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's <" << it->getUlamTypeName(this).c_str() << "> bit size: <" << it->getBitSize() << "> does not match resulting type's <" << rType->getUlamTypeName(this).c_str() << "> bit size: <" << rType->getBitSize() << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturnStatement", "checkAndLabelType", -1, MSG_ERR);
		  }
	      } //base types are the same..array and bit size might vary
	  } //different ulamtype
      } //next return node

    return rtnBool;
  }


  void CompilerState::indent(File * fp)
  {
    for(u32 i = 0; i < m_currentIndentLevel; i++)
      {
	fp->write(m_indentedSpaceLevel); 
      }
  }

  std::string CompilerState::getFileNameForAClassHeader(u32 id)
  {
    std::ostringstream f;
    Symbol * csym = m_programDefST.getSymbolPtr(id);
    UlamType * cut = csym->getUlamType();
    f << cut->getUlamTypeMangledName(this).c_str() << ".h";
    return f.str();
  }


  std::string CompilerState::getFileNameForThisClassHeader()
  {
    return getFileNameForAClassHeader(m_compileThisId);
  }


  std::string CompilerState::getFileNameForThisClassBody()
  {
    std::ostringstream f;
    Symbol * csym = m_programDefST.getSymbolPtr(m_compileThisId);
    UlamType * cut = csym->getUlamType();
    f << cut->getUlamTypeMangledName(this).c_str() << ".cpp";
    return f.str();
  }


  //separate file for element compilations, avoid multiple mains, select the one to test during linking
  std::string CompilerState::getFileNameForThisClassMain()
  {
    std::ostringstream f;
    Symbol * csym = m_programDefST.getSymbolPtr(m_compileThisId);
    UlamType * cut = csym->getUlamType();
    f << cut->getUlamTypeMangledName(this).c_str() << "_main.cpp";
    return f.str();
  }


  std::string CompilerState::getFileNameForThisTypesHeader()
  {
    std::ostringstream f;
    Symbol * csym = m_programDefST.getSymbolPtr(m_compileThisId);
    UlamType * cut = csym->getUlamType();
    f << cut->getUlamTypeMangledName(this).c_str() << "_Types.h";
    return f.str();
  }


} //end MFM
