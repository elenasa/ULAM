#include <set>
#include "SymbolFunctionName.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

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
    //assert(getUlamTypeIdx() == fsym->getUlamTypeIdx());
    if(UlamType::compare(getUlamTypeIdx(), fsym->getUlamTypeIdx(), m_state) != UTIC_SAME)
      {
	std::ostringstream msg;
	msg << "Overloading Function: " << m_state.m_pool.getDataAsString(fsym->getId()).c_str() << " Returns DIFFERENT type: '" << m_state.getUlamTypeNameByIndex(fsym->getUlamTypeIdx()).c_str() << "' (UTI" << fsym->getUlamTypeIdx() << ") than " << m_state.getUlamTypeNameByIndex(getUlamTypeIdx()).c_str() << "' (UTI" << getUlamTypeIdx() << ")";
	MSG("", msg.str().c_str(), ERR);
	return false;
      }

    //std::string mangled = fsym->getMangledNameWithTypes();
    std::string mangled = fsym->getMangledNameWithUTIparameters();

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
  } //overloadFunction


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
  } //findMatchingFunction


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
  } //getDepthSumOfFunctions


  u32 SymbolFunctionName::checkFunctionNames()
  {
    u32 probcount = 0;
    std::set<std::string>mangledFunctionSet; //detect duplicated UTI parameters
    std::vector<std::string>dupfuncs;

    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	std::string fkey = it->first;
	SymbolFunction * fsym = it->second;
	std::string fmangled = fsym->getMangledNameWithTypes();
	std::pair<std::set<std::string>::iterator, bool> ret;
	ret = mangledFunctionSet.insert(fmangled);
	bool overloaded = ret.second; //false if already existed, i.e. not added
	if(!overloaded) //shouldn't be a duplicate
	  {
	    std::ostringstream msg;
	    msg << "Check overloading function: <" << m_state.m_pool.getDataAsString(fsym->getId()).c_str() << "> (" << fkey.c_str() << "), has a duplicate definition: " << fmangled.c_str();
	    MSG("", msg.str().c_str(), ERR);
	    probcount++;
	    dupfuncs.push_back(fkey);
	  }
	++it;
      }
    mangledFunctionSet.clear(); //strings only

    while(!dupfuncs.empty())
      {
	std::string dupkey = dupfuncs.back();
	std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.find(dupkey);

	if(it != m_mangledFunctionNames.end())
	  {
	    assert(dupkey == it->first);
	    SymbolFunction * fsym = it->second;
	    delete fsym;
	    it->second = NULL;
	    m_mangledFunctionNames.erase(it);
	  }
	dupfuncs.pop_back();
      }

    dupfuncs.clear();
    return probcount;
  } //labelFunctions


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
  } //labelFunctions


  u32 SymbolFunctionName::countNativeFuncDecls()
  {
    u32 count = 0;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	count += fsym->isNativeFunctionDeclaration();
	++it;
      }
    return count;
  } //countNativeFuncDecls


  void SymbolFunctionName::generateCodedFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	fsym->generateFunctionDeclaration(fp, declOnly, classtype);
	++it;
      }
  } //generateCodedFunctions


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
  } //isDefined

} //end MFM
