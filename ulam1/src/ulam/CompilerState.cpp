#include <stdio.h>
#include <iostream>
#include "SymbolVariable.h"
#include "SymbolTypedef.h"
#include "CompilerState.h"
#include "UlamTypeNav.h"
#include "UlamTypeVoid.h"
#include "UlamTypeInt.h"
#include "UlamTypeFloat.h"
#include "UlamTypeBool.h"
#include "UlamTypeElement.h"
#include "UlamTypeQuark.h"
#include "UlamTypeAtom.h"
#include "NodeBlockClass.h"


//#define _DEBUG_OUTPUT 
#ifdef _DEBUG_OUTPUT
static const bool debugOn = true;
#else
static const bool debugOn = false;
#endif

namespace MFM {

  CompilerState::CompilerState(): m_currentBlock(NULL), m_classBlock(NULL), m_currentFunctionBlockDeclSize(0),
				  m_currentFunctionBlockMaxDepth(0)
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
    std::string typeName  = getTokenAsATypeName(typeTok); //Foo, Int, etc

    // is this name already a typedef for a complex type?
    ULAMTYPE bUT = getBaseTypeFromToken(typeTok);

    bitsize = (bitsize == 0 ? (bUT == Bool ? 8 : 32) : bitsize); //temporary!!!
    UlamKeyTypeSignature key(typeName.c_str(),bitsize,arraysize);
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
      case Element:
	ut = new UlamTypeElement(key, uti);      
	break;
      case Quark:
	ut = new UlamTypeQuark(key, uti);      
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


  const char * CompilerState::getUlamTypeNameByIndex(UTI uti)
  {
    return m_indexToUlamType[uti]->getUlamTypeNameBrief();
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
	//no way to get the bUT, except to assume typeName is one of them?
	bUT = UlamType::getEnumFromUlamTypeString(typeName.c_str()); //could be Element, etc.;
      }
    return bUT;
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
    //else //must be a primitive, but could be array..
    //  {
	//UTI bUT = UlamType::getEnumFromUlamTypeString(name); //could be Element, etc.;
	//rtnType = getUlamTypeByIndex(bUT);
	//rtnBool = true;
    //  }

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
    //const char * name = keyOfArg.getUlamKeyTypeSignatureName();
    const char * name = UlamType::getUlamTypeEnumAsString(bUT);
    UlamKeyTypeSignature baseKey(name, bitsize);  //default array size is zero
    //ULAMTYPE bUT = utArg->getEnumFromUlamTypeString(name);
    UTI buti = makeUlamType(baseKey, bUT);
    return getUlamTypeByIndex(buti);
  }


  bool CompilerState::alreadyDefinedSymbol(u32 dataindex, Symbol * & symptr)
  {
    bool brtn = false;

    // start with the current "top" block and look down the stack
    // until the 'variable id' is found.
    NodeBlock * blockNode = m_currentBlock;
    while(!brtn && blockNode)
      {
	brtn = blockNode->isIdInScope(dataindex,symptr);
	blockNode = blockNode->getPreviousBlockPointer();  //traverse the chain
      }

    // data member variables in class block are linked to the function block.
    // check function data members separately
    if(!brtn)
      {
	brtn = m_classBlock->isFuncIdInScope(dataindex,symptr);
      }

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
    if(Token::isTokenAType(tok,this))
      {
	if((Token::getSpecialTokenWork(tok.m_type) == TOKSP_TYPEKEYWORD))
	  {
	    return std::string(Token::getTokenAsString(tok.m_type));
	  }
	
 	return getDataAsString(&tok);
      }

    return "Nav";
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
	NodeReturn * rNode = m_currentFunctionReturnNodes.at(i);
	UlamType * rType = rNode->getNodeType();
	
	if(rType != it)
	  {
	    rtnBool = false;

	    ULAMTYPE rBUT = rType->getUlamTypeEnum();
	    ULAMTYPE itBUT = it->getUlamTypeEnum();
	    if(rBUT != itBUT)
	      {
		std::ostringstream msg;
		msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's <" << it->getUlamTypeName().c_str() << "> base type: <" << UlamType::getUlamTypeEnumAsString(itBUT) << "> does not match resulting type's <" << rType->getUlamTypeName().c_str() << "> base type: <" << UlamType::getUlamTypeEnumAsString(rBUT) << ">";
		m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturn", "checkAndLabelType", -1, MSG_ERR);

	      }
	    else
	      {
		if(rType->getArraySize() != it->getArraySize())
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's <" << it->getUlamTypeName().c_str() << "> array size: <" << it->getArraySize() << "> does not match resulting type's <" << rType->getUlamTypeName().c_str() << "> array size: <" << rType->getArraySize() << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturn", "checkAndLabelType", -1, MSG_ERR);
		  }

		if(rType->getBitSize() != it->getBitSize())
		  {
		    std::ostringstream msg;
		    msg << "Function '" << m_pool.getDataAsString(fsym->getId()).c_str() << "''s Return type's <" << it->getUlamTypeName().c_str() << "> bit size: <" << it->getBitSize() << "> does not match resulting type's <" << rType->getUlamTypeName().c_str() << "> bit size: <" << rType->getBitSize() << ">";
		    m_err.buildMessage(rNode->getNodeLocationAsString().c_str(), msg.str().c_str(), "MFM::NodeReturn", "checkAndLabelType", -1, MSG_ERR);
		  }
		
	      } //base types are the same..array and bit size might vary
	  } //different ulamtype
      } //next return node

    return rtnBool;
  }


} //end MFM
