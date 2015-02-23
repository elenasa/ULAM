#include <set>
#include "SymbolFunctionName.h"
#include "Node.h"
#include "NodeBlockClass.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"
#include "SymbolTable.h"

namespace MFM {

  SymbolFunctionName::SymbolFunctionName(u32 id, UTI typetoreturn, CompilerState& state) : Symbol(id,typetoreturn,state)
  {
    setDataMember(); // by definition all function definitions are data members
  }

  SymbolFunctionName::SymbolFunctionName(const SymbolFunctionName& sref) : Symbol(sref)
  {
    std::map<std::string, SymbolFunction *>::const_iterator it = sref.m_mangledFunctionNames.begin();
    while(it != sref.m_mangledFunctionNames.end())
      {
	SymbolFunction * foundSym = it->second;
	SymbolFunction * cloneOf = (SymbolFunction *) foundSym->clone();
	// return types could be mapped UTI, and not the same
	//assert(foundSym != cloneOf && foundSym->getUlamTypeIdx() == cloneOf->getUlamTypeIdx());
	//std::string clonemangled(it->first);
	//m_mangledFunctionNames.insert(std::pair<std::string,SymbolFunction *>(clonemangled,cloneOf));
	overloadFunction(cloneOf);
	++it;
      }
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

  Symbol * SymbolFunctionName::clone()
  {
    return new SymbolFunctionName(*this);
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
    // return types may differ, as long as params are different
    //assert(getUlamTypeIdx() == fsym->getUlamTypeIdx());

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

  // before generating code, remove duplicate funcs to avoid "previously declared" gcc error.
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
	if(!overloaded) //shouldn't be a duplicate, but if it is handle it
	  {
	    std::ostringstream msg;
	    msg << "Check overloading function: <" << m_state.m_pool.getDataAsString(fsym->getId()).c_str() << "> has a duplicate definition: " << fmangled.c_str() << ", while compiling class: " << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG("", msg.str().c_str(), ERR);  //Dave says better to start as error
	    probcount++;
	    dupfuncs.push_back(fkey);
	  }
	++it;
      }
    mangledFunctionSet.clear(); //strings only

    //unclear which dup function is found/removed; case of more than one dup is handled similarly;
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
  } //checkFunctionNames

  u32 SymbolFunctionName::checkCustomArrayFunctions(SymbolTable & fST)
  {
    u32 probcount = 0;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    //loop over 'aref's for return type; then check for 'aset' existence/correctness
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	UTI futi = fsym->getUlamTypeIdx();
	assert(futi != Void);

	// CANT use UTI directly, must key to compare as they may change.???
	// check that its 'set' function has a matching parameter type
	Symbol * fnsymset = NULL;
	if(fST.isInTable(m_state.getCustomArraySetFunctionNameId(), fnsymset))
	  {
	    SymbolFunction * funcSymbol = NULL;
	    std::vector<UTI> argTypes;
	    argTypes.push_back(Int);
	    argTypes.push_back(futi);
	    if(!((SymbolFunctionName *) fnsymset)->findMatchingFunction(argTypes, funcSymbol))
	      {
		std::ostringstream msg;
		msg << "Custom array set method: '" << m_state.m_pool.getDataAsString(fnsymset->getId()).c_str() << "' does not match '" << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str() << "' argument types: ";
		for(u32 i = 0; i < argTypes.size(); i++)
		  {
		    msg << m_state.getUlamTypeNameByIndex(argTypes[i]).c_str() << ", ";
		  }
		msg << "and cannot be called in class: " << m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->getUlamTypeNameOnly().c_str();
		MSG("", msg.str().c_str(), ERR);
		probcount++;
	      }
	    else
	      {
		std::ostringstream msg;
		msg << "Custom array set method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArraySetFunctionNameId()).c_str() << "' FOUND in class: " << m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->getUlamTypeNameOnly().c_str();
		MSG("", msg.str().c_str(), DEBUG);
	      }
	    argTypes.clear();
	  }//aset found
	else
	  {
	    std::ostringstream msg;
	    msg << "Custom array set method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArraySetFunctionNameId()).c_str() << "' NOT FOUND in class: " << m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->getUlamTypeNameOnly().c_str();
	    MSG("", msg.str().c_str(), WARN);
	  }
	++it;
      } //while get found
    return probcount;
  } //checkCustomArrayFunctions

  void SymbolFunctionName::linkToParentNodesInFunctionDefs(NodeBlockClass * p)
  {
    NNO pno = p->getNodeNo();
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body? perhaps an ACCESSOR to-be-made?
	func->updateLineage(pno);
	++it;
      }
  } //linkToParentNodesInFunctionDefs

  bool SymbolFunctionName::findNodeNoInFunctionDefs(NNO n, Node*& foundNode)
  {
    bool rtnfound = false;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body? perhaps an ACCESSOR to-be-made?
	if(func->findNodeNo(n, foundNode))
	  {
	    rtnfound = true;
	    break;
	  }
	++it;
      }
    return rtnfound;
  }//findNodeNoInFunctionDefs

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

  u32 SymbolFunctionName::countNavNodesInFunctionDefs()
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    u32 countNavs = 0;

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func);

	u32 fcntnavs = 0;
	func->countNavNodes(fcntnavs);
	if(fcntnavs > 0)
	  {
	    std::string fkey = it->first;
	    std::ostringstream msg;
	    msg << fcntnavs << " nodes with illegal 'Nav' types remain in function <";
	    msg << m_state.m_pool.getDataAsString(getId());
	    msg << "> (" << fkey.c_str() << ")";
	    MSG(func->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
	countNavs += fcntnavs;
	++it;
      }

    if(countNavs > 0)
      {
	std::ostringstream msg;
	msg << countNavs << " nodes with illegal 'Nav' types remain in all functions <";
	msg << m_state.m_pool.getDataAsString(getId());
	msg << "> in class: " << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG("", msg.str().c_str(), WARN);
      }
#if 0
    else
      {
	std::ostringstream msg;
	msg << countNavs << " nodes with illegal 'Nav' types remain in functions <";
	msg << m_state.m_pool.getDataAsString(getId());
	msg << "> --- good to go!";
	MSG("", msg.str().c_str(), DEBUG);
      }
#endif
    return countNavs;
  } //countNavNodesInFunctionDefs

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
