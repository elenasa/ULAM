#include "SymbolFunctionName.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"

namespace MFM {

  SymbolFunctionName::SymbolFunctionName(u32 id, UTI typetoreturn, CompilerState& state) : Symbol(id,typetoreturn,state)
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


  bool SymbolFunctionName::overloadFunction(SymbolFunction * fsym)
  {
    bool overloaded = false;

    assert(getUlamTypeIdx() == fsym->getUlamTypeIdx());

    std::string mangled = fsym->getMangledNameWithTypes();
   
    //if doesn't already exist, potentially overload it by inserting into map.
    SymbolFunction * anyotherSym;
    if(!isDefined(mangled, anyotherSym))
      {
	std::pair<std::map<std::string,SymbolFunction *>::iterator,bool> ret;
	ret = m_mangledFunctionNames.insert(std::pair<std::string,SymbolFunction *>(mangled,fsym));
	overloaded = ret.second; //false if already existed, i.e. not added
	assert(overloaded); //shouldn't be a duplicate, we've checked by now.
      }

    return overloaded;
  }


  bool SymbolFunctionName::findMatchingFunction(std::vector<UTI> argTypes, SymbolFunction *& funcSymbol)
  {
    bool rtnBool = false;

    if(m_mangledFunctionNames.empty())
      return false;

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
	assert(func); //how would a function symbol be without a body? perhaps an ACCESSOR to-be-made?
	func->checkAndLabelType();
	++it;
      }
  }


  void SymbolFunctionName::generateCodedFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;  
	fsym->generateFunctionDeclaration(fp, declOnly, classtype);  
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
