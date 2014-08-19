#include "SymbolFunctionName.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"

namespace MFM {

  SymbolFunctionName::SymbolFunctionName(u32 id, UlamType * typetoreturn) : Symbol(id,typetoreturn)
  { 
    setDataMember(); // by definition all function definitions are data members
  }

  SymbolFunctionName::~SymbolFunctionName()
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * foundSym = it->second;  
	delete foundSym;   //further deletes its Node func def
	++it;
      }

    m_mangledFunctionNames.clear();
  }


  bool SymbolFunctionName::isFunction()
  {
    return true;
  }


  const std::string SymbolFunctionName::getMangledPrefix()
  {
    return "Uz_";  //?
  }


  bool SymbolFunctionName::overloadFunction(SymbolFunction * fsym, CompilerState& state)
  {
    bool overloaded = false;

    assert(getUlamType() == fsym->getUlamType());

    std::string mangled = fsym->getMangledNameWithTypes(state);
   
    //if doesn't already exist, potentially overload it by inserting into map.
    SymbolFunction * anyotherSym;
    if(!isDefined(mangled, anyotherSym))
      {
	m_mangledFunctionNames.insert(std::pair<std::string,SymbolFunction *>(mangled,fsym));
	overloaded = true;
      }

    return overloaded;
  }


  bool SymbolFunctionName::findMatchingFunction(std::vector<UlamType *> argTypes, SymbolFunction *& funcSymbol)
  {
    bool rtnBool = false;

    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    assert(funcSymbol == NULL);

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;  
	if((rtnBool = fsym->matchingTypes(argTypes)))
	  {
	    funcSymbol = fsym;
	    break;
	  }
	++it;
      }

    return rtnBool;
  }


  u32 SymbolFunctionName::getDepthSumOfFunctions()
  {
    u32 depthsum = 0;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;  
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body?
	depthsum += func->getMaxDepth();
	++it;
      }
    return depthsum;
  }


  void SymbolFunctionName::labelFunctions()
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;  
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body?
	func->checkAndLabelType();
	++it;
      }
  }


  void SymbolFunctionName::generateCodedFunctions(File * fp)
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;  
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body?
	func->genCode(fp);
	++it;
      }
  }


  //private method:
  bool SymbolFunctionName::isDefined(std::string mangledFName, SymbolFunction * & foundSym)
  {
    bool rtnBool= false;

    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.find(mangledFName);

    if(it != m_mangledFunctionNames.end())
      {
	foundSym = it->second;  
	rtnBool = true;
      }

    return rtnBool;
  }


} //end MFM
