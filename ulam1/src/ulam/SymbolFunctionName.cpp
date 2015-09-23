#include <set>
#include "SymbolFunctionName.h"
#include "Node.h"
#include "NodeBlockClass.h"
#include "NodeBlockFunctionDefinition.h"
#include "SymbolVariable.h"
#include "CompilerState.h"
#include "SymbolTable.h"
#include "MapFunctionDesc.h"

namespace MFM {

  SymbolFunctionName::SymbolFunctionName(Token id, UTI typetoreturn, CompilerState& state) : Symbol(id,typetoreturn,state)
  {
    setDataMember(); //by definition all function definitions are data members
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
    return "Uz_"; //?
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

  u32 SymbolFunctionName::findMatchingFunction(std::vector<UTI> argTypes, SymbolFunction *& funcSymbol)
  {
    if(m_mangledFunctionNames.empty())
      return 0;

    u32 matchingFuncCount = 0;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    assert(funcSymbol == NULL);

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	if(fsym->matchingTypesStrictly(argTypes))
	  {
	    funcSymbol = fsym;
	    matchingFuncCount++;
	    //break;
	  }
	++it;
      }
    return matchingFuncCount;
  } //findMatchingFunction

  u32 SymbolFunctionName::findMatchingFunctionWithConstantsAsArgs(std::vector<UTI> argTypes, std::vector<Node*> constArgs, SymbolFunction *& funcSymbol, bool& hasHazyArgs)
  {
    assert(!hasHazyArgs);

    if(m_mangledFunctionNames.empty())
      return 0;

    u32 matchingFuncCount = 0;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    assert(funcSymbol == NULL);

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	if(fsym->matchingTypesStrictly(argTypes))
	  {
	    funcSymbol = fsym;
	    matchingFuncCount++;
	  }
	++it;
      }

    //try again with less strict constraints, allow safe casting;
    //track matches with hazy casting for error message output
    if(!funcSymbol)
      {
	// give priority to safe matches that have same ULAMTYPEs too
	// e.g. Unsigned(3) arg would safely cast to Int(4), Int(5), and Unsigned param;
	//      this is ambiguous! but only one is the same Enum; so let's
	//      call it the winner.
	SymbolFunction * funcSymbolMatchingUTArgs = NULL;
	u32 numFuncsWithAllSameUTArgs = 0;
	u32 numArgs = argTypes.size();

	it = m_mangledFunctionNames.begin();
	while(it != m_mangledFunctionNames.end())
	  {
	    SymbolFunction * fsym = it->second;
	    u32 numUTmatch = 0;
	    if(fsym->matchingTypes(argTypes, constArgs, hasHazyArgs, numUTmatch))
	      {
		funcSymbol = fsym;
		matchingFuncCount++;
		if(numUTmatch == numArgs)
		  {
		    numFuncsWithAllSameUTArgs++;
		    funcSymbolMatchingUTArgs = funcSymbol;
		  }
	      }
	    ++it;
	  }

	if(matchingFuncCount > 1)
	  {
	    if(numFuncsWithAllSameUTArgs > 1)
	      funcSymbol = NULL;
	    else
	      {
		//instead of null, let's go with the one/none closest in safe matching args
		matchingFuncCount = numFuncsWithAllSameUTArgs; // ==0 or 1
		funcSymbol = funcSymbolMatchingUTArgs;
	      }
	  }
      } //2nd try
    return matchingFuncCount;
  } //findMatchingFunctionWithConstantsAsArgs

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

  // after all the UTI types are known, including array and bitsize
  // and before eval() for testing, update the maxdepth that was
  // calculated during parsetime
  void SymbolFunctionName::calcMaxDepthOfFunctions()
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body?
	u32 depth = 0;
	u32 maxdepth = 0;
	s32 base = 0;
	//add size of parameters (+ 1 for hidden) and return size
	base += m_state.slotsNeeded(fsym->getUlamTypeIdx()); //return type
	base += fsym->getTotalParameterSlots();
	base += 1; //hidden arg is a symbol, not a node
	func->calcMaxDepth(depth, maxdepth, base);
	maxdepth += 1; //hidden, (frame ptr at zero implicit)
	func->setMaxDepth(maxdepth);
	++it;
      }
    return;
  } //calcMaxDepthOfFunctions

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
	    msg << "Check overloading function <";
	    msg << m_state.m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "> has a duplicate definition (" << fmangled.c_str();
	    msg << "), while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(fsym->getTokPtr(), msg.str().c_str(), ERR);  //Dave says better to start as error
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

  u32 SymbolFunctionName::checkCustomArrayGetFunctions(UTI& rtnType)
  {
    u32 probcount = 0;
    UTI futisav = Nav;

    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    //loop over 'aref's for return type; then check for 'aset' existence/correctness
    // aref's should all return the same type
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	UTI futi = fsym->getUlamTypeIdx();
	if(futi == Void)
	  {
	    std::ostringstream msg;
	    msg << "Custom array get method '";
	    msg << m_state.m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "' cannot return Void ";
	    probcount++;
	  }
	else if(futisav != Nav && UlamType::compare(futi, futisav, m_state) == UTIC_NOTSAME)
	  {
	    std::ostringstream msg;
	    msg << "Custom array get methods '";
	    msg << m_state.m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "' cannot have different return types: ";
	    msg << m_state.getUlamTypeNameByIndex(futi).c_str();
	    msg << ", ";
	    msg << m_state.getUlamTypeNameByIndex(futisav).c_str();
	    probcount++;
	  }
	futisav = futi;
	it++;
      } //while

    if(!probcount)
      rtnType = futisav;
    else
      rtnType = Nav;
    return probcount;
  } //checkCustomArrayGetFunctions

  u32 SymbolFunctionName::checkCustomArraySetFunctions(UTI caType)
  {
    if(m_mangledFunctionNames.empty())
      {
	std::ostringstream msg;
	msg << "Custom array set method '";
	msg << m_state.m_pool.getDataAsString(m_state.getCustomArraySetFunctionNameId()).c_str();
	msg << "' NOT FOUND in class: ";
	msg << m_state.getUlamTypeByIndex(m_state.getCompileThisIdx())->getUlamTypeNameOnly().c_str();
	MSG(getTokPtr(), msg.str().c_str(), INFO);
	return 0;
      }

    u32 probcount = 0;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	UTI futi = fsym->getUlamTypeIdx();
	if(futi != Void)
	  {
	    std::ostringstream msg;
	    msg << "Custom array set method '";
	    msg << m_state.m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "' must return Void ";
	    probcount++;
	  }

	u32 numparams = fsym->getNumberOfParameters();
	if(numparams != 2)
	  {
	    std::ostringstream msg;
	    msg << "Custom array set method '";
	    msg << m_state.m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "' requires exactly two parameters, not ";
	    msg << numparams;
	    probcount++;
	  }
	else
	  {
	    //verify 2nd parameter matches caType
	    // compare UTI as they may change during full instantiations
	    Symbol * asym = fsym->getParameterSymbolPtr(1);
	    assert(asym);
	    UTI auti = asym->getUlamTypeIdx();
	    if(UlamType::compare(auti, caType, m_state) == UTIC_NOTSAME)
	      {
		std::ostringstream msg;
		msg << "Custom array set method '";
		msg << m_state.m_pool.getDataAsString(fsym->getId()).c_str();
		msg << "' second parameter type '";
		msg << m_state.getUlamTypeNameByIndex(auti).c_str();
		msg << "' does not match '";
		msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str();
		msg << "' return type '";
		msg << m_state.getUlamTypeNameByIndex(caType).c_str();
		msg << "'; Cannot be called in class: ";
		msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
		MSG(fsym->getTokPtr(), msg.str().c_str(), ERR);
		probcount++;
	      }
	  }
	it++;
      } //while aset found
    return probcount;
  } //checkCustomArraySetFunctions

  UTI SymbolFunctionName::getCustomArrayReturnType()
  {
    UTI rtnType = Nav;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    //loop over 'aref's for return type;
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	UTI futi = fsym->getUlamTypeIdx();
	assert(futi != Void);
	assert(rtnType == Nav || UlamType::compare(rtnType, futi, m_state) == UTIC_SAME);
	rtnType = futi;
	++it;
      }
    return rtnType;
  } //getCustomArrayReturnType

  //similar to
  u32 SymbolFunctionName::getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    std::vector<UTI> argTypes;
    std::vector<Node *> constArgs;
    argTypes.push_back(rnode->getNodeType());
    constArgs.push_back(rnode->isAConstant() ? rnode : NULL);

    SymbolFunction * fsym = NULL;
    u32 camatches = findMatchingFunctionWithConstantsAsArgs(argTypes, constArgs, fsym, hasHazyArgs);

    if(camatches == 1)
      {
	Symbol * asym = fsym->getParameterSymbolPtr(0); //1st arg is index
	assert(asym);
	idxuti = asym->getUlamTypeIdx();
      }

    argTypes.clear();
    constArgs.clear();
    return camatches;
  } //getCustomArrayIndexTypeFor

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

  void SymbolFunctionName::updatePrevBlockPtrInFunctionDefs(NodeBlockClass * p)
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body? perhaps an ACCESSOR to-be-made?
	func->setPreviousBlockPointer(p);
	++it;
      }
  } //updatePrevBlockPtrInFunctionDefs

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
  } //findNodeNoInFunctionDefs

  bool SymbolFunctionName::labelFunctions()
  {
    bool aok = true; //parameter types

    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;

	// now done as part of block's c&l
	// first check for incomplete parameters
	//aok &= fsym->checkParameterTypes();

	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body? perhaps an ACCESSOR to-be-made?
	func->checkAndLabelType();
	++it;
      }
    return aok;
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
	    msg << fcntnavs << " nodes with unresolved types remain in function <";
	    msg << m_state.m_pool.getDataAsString(getId());
	    msg << "> (" << fkey.c_str() << ")";
	    MSG(func->getNodeLocationAsString().c_str(), msg.str().c_str(), WARN);
	  }
	countNavs += fcntnavs;
	++it;
      }

    if(countNavs > 0)
      {
	u32 numfuncs = m_mangledFunctionNames.size();
	std::ostringstream msg;
	msg << "Summary: " << countNavs << " nodes with unresolved types remain in ";
	if(numfuncs > 1)
	  msg << "all " <<  numfuncs << " functions <";
	else
	  msg << " a single function <";
	msg << m_state.m_pool.getDataAsString(getId()) << "> in class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(m_state.getFullLocationAsString(getLoc()).c_str(), msg.str().c_str(), WARN);
      }
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

  void SymbolFunctionName::addFunctionDescriptionsToClassMemberMap(UTI classType, ClassMemberMap & classmembers)
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	FunctionDesc * descptr = new FunctionDesc(fsym, classType, m_state);
	assert(descptr);

	//concat mangled class and parameter names to avoid duplicate keys into map
	std::ostringstream fullMangledName;
	fullMangledName << descptr->m_mangledClassName << "_" << descptr->m_mangledMemberName;
	classmembers.insert(std::pair<std::string, struct ClassMemberDesc *>(fullMangledName.str(), descptr));
	++it;
      }
  } //addFunctionDescriptionsToClassMemberMap

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
