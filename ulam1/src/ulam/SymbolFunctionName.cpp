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

  SymbolFunctionName::SymbolFunctionName(Token id, UTI typetoreturn, CompilerState& state) : Symbol(id, typetoreturn, state)
  {
    setDataMemberClass(m_state.getCompileThisIdx()); //by definition all function definitions are data members
  }

  SymbolFunctionName::SymbolFunctionName(const SymbolFunctionName& sref) : Symbol(sref)
  {
    std::map<std::string, SymbolFunction *>::const_iterator it = sref.m_mangledFunctionNames.begin();
    while(it != sref.m_mangledFunctionNames.end())
      {
	SymbolFunction * foundSym = it->second;
	SymbolFunction * cloneOf = (SymbolFunction *) foundSym->clone();
	// return types could be mapped UTI, and not the same
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

  u32 SymbolFunctionName::findMatchingFunctionStrictlyByTypes(std::vector<UTI> argTypes, SymbolFunction *& funcSymbol)
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
  } //findMatchingFunctionStrictlyByTypes

  u32 SymbolFunctionName::findMatchingFunction(std::vector<Node *> argNodes, SymbolFunction *& funcSymbol)
  {
    if(m_mangledFunctionNames.empty())
      return 0;

    u32 matchingFuncCount = 0;
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    assert(funcSymbol == NULL);

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	if(fsym->matchingTypesStrictly(argNodes))
	  {
	    funcSymbol = fsym;
	    matchingFuncCount++;
	    //break;
	  }
	++it;
      }
    return matchingFuncCount;
  } //findMatchingFunction

  u32 SymbolFunctionName::findMatchingFunctionWithSafeCasts(std::vector<Node *> argNodes, SymbolFunction *& funcSymbol, bool& hasHazyArgs)
  {
    assert(!hasHazyArgs);
    assert(!funcSymbol);

    if(m_mangledFunctionNames.empty())
      return 0;

    u32 matchingFuncCount = findMatchingFunction(argNodes, funcSymbol); //strictly first

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
	u32 numArgs = argNodes.size();

	std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
	while(it != m_mangledFunctionNames.end())
	  {
	    SymbolFunction * fsym = it->second;
	    u32 numUTmatch = 0;
	    if(fsym->matchingTypes(argNodes, hasHazyArgs, numUTmatch)) //with safe casting
	      {
		funcSymbol = fsym;
		matchingFuncCount++;
		if(numUTmatch == numArgs)
		  {
		    numFuncsWithAllSameUTArgs++;
		    funcSymbolMatchingUTArgs = funcSymbol;
		  }
	      }
	    it++;
	  } //end while

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

    //3rd try: check any super class, unless hazyargs (causes inf loop)
    if(matchingFuncCount == 0)
	return findMatchingFunctionWithSafeCastsInAncestors(argNodes, funcSymbol, hasHazyArgs);

    return matchingFuncCount;
  } //findMatchingFunctionWithConstantsAsArgs

  u32 SymbolFunctionName::findMatchingFunctionWithSafeCastsInAncestors(std::vector<Node *> argNodes, SymbolFunction *& funcSymbol, bool& hasHazyArgs)
  {
    Symbol * fnsym = NULL;
    UTI cuti = m_state.findAClassByNodeNo(getBlockNoOfST());
    assert(cuti != Nav);
    UTI supercuti = m_state.isClassASubclass(cuti);
    if(supercuti != Nouti)
      {
	if(m_state.isFuncIdInAClassScope(supercuti, getId(), fnsym, hasHazyArgs) && !hasHazyArgs)
	  return ((SymbolFunctionName *) fnsym)->findMatchingFunctionWithSafeCasts(argNodes, funcSymbol, hasHazyArgs); //recurse ancestors
      }
    return 0;
  } //findMatchingFunctionWithSafeCastsInAncestors

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

  // after all the UTI types are known, including array and bitsize
  // and before eval() for testing, calc the max index of virtual functions,
  // where base class functions come first; arg is UNKNOWNSIZE if first time
  // and any ancestors are already known.
  void SymbolFunctionName::calcMaxIndexOfVirtualFunctions(s32& maxidx)
  {
    UTI cuti = m_state.getCompileThisIdx();
    u32 fid = getId();
    UTI superuti = m_state.isClassASubclass(cuti);

    //initialize this classes VTable to super classes' VTable, or empty
    // some entries may be modified; or table may expand
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;

	//possibly us, or a great-ancestor that has first decl of this func
	UTI kinuti; // ref to find, Nav if not found
	s32 vidx = UNKNOWNSIZE; //virtual index

	//search for virtual function w exact name/type in superclass
	// if found, this function must also be virtual
	assert(superuti != Hzy);
	if(superuti != Nouti)
	  {
	    std::vector<UTI> pTypes;
	    u32 numparams = fsym->getNumberOfParameters();
	    for(u32 j = 0; j < numparams; j++)
	      {
		Symbol * psym = fsym->getParameterSymbolPtr(j);
		assert(psym);
		pTypes.push_back(psym->getUlamTypeIdx());
	      }

	    SymbolFunction * superfsym = NULL;
	    //might belong to a great-ancestor (can't tell from isFuncIdInAClassScope())
	    if(m_state.findMatchingFunctionInAncestor(cuti, fid, pTypes, superfsym, kinuti))
	      {
		if(superfsym->isVirtualFunction())
		  {
		    //like c++, this fsym should be too!
		    if(!fsym->isVirtualFunction())
		      {
			std::ostringstream msg;
			msg << "Non-virtual overloaded function <";
			msg << m_state.m_pool.getDataAsString(fid).c_str();
			msg << "> has a VIRTUAL ancestor in class: ";
			msg << m_state.getUlamTypeNameBriefByIndex(kinuti).c_str();
			MSG(fsym->getTokPtr(), msg.str().c_str(), WARN);

			fsym->setVirtualFunction(); //fix
			assert(maxidx != UNKNOWNSIZE); //o.w. wouldn't be here yet
		      }
		    vidx = superfsym->getVirtualMethodIdx();
		    kinuti = cuti; //overriding
		  }
		else
		  {
		    if(fsym->isVirtualFunction())
		      {
			//c++, quietly fails
			std::ostringstream msg;
			msg << "Virtual overloaded function <";
			msg << m_state.m_pool.getDataAsString(fid).c_str();
			msg << "> has a NON-VIRTUAL ancestor in class: ";
			msg << m_state.getUlamTypeNameBriefByIndex(kinuti).c_str();
			MSG(fsym->getTokPtr(), msg.str().c_str(), WARN);
			//probably upsets compiler assert...need a Nav node?
			kinuti = cuti;
		      }
		  }
	      }
	    else
	      kinuti = cuti; //new entry, this class
	  } //end ancestor check
	else
	  kinuti = cuti; //new entry, this class

	if(fsym->isVirtualFunction())
	  {
	    if(vidx == UNKNOWNSIZE)
	      {
		//table extends with new (next) entry/idx
		vidx = (maxidx != UNKNOWNSIZE ? maxidx : 0);
		maxidx = vidx + 1;
	      }
	    //else use ancestor index; maxidx stays same
	    fsym->setVirtualMethodIdx(vidx);
	    csym->updateVTable(vidx, fsym, kinuti, fsym->isPureVirtualFunction());
	  }
	else
	  maxidx = (maxidx != UNKNOWNSIZE ? maxidx : 0); //stays same, or known 0
	++it;
      } //while
    return;
  } //calcMaxIndexOfVirtualFunctions

  void SymbolFunctionName::checkAbstractInstanceErrorsInFunctions()
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body?
	func->checkAbstractInstanceErrors();
	++it;
      }
    return;
  } //checkAbstractInstanceErrorsInFunctions

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
	    msg << "Check overloaded function <";
	    msg << m_state.m_pool.getDataAsString(fsym->getId()).c_str();
	    msg << "> has a duplicate definition (" << fmangled.c_str();
	    msg << "), while compiling class: ";
	    msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	    MSG(fsym->getTokPtr(), msg.str().c_str(), ERR);  //Dave says better to start as error
	    NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	    assert(func);
	    func->setNodeType(Nav); //compiler counts
	    probcount++;
	    dupfuncs.push_back(fkey);
	  }
	++it;
      }
    mangledFunctionSet.clear(); //strings only

    //no longer try to erase the dup function; prefer the error msg Thu Jan 21 11:24:07 2016
    //unclear which dup function is found/removed; case of more than one dup is handled similarly;
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
	else if(m_state.okUTItoContinue(futisav) && (UlamType::compare(futi, futisav, m_state) == UTIC_NOTSAME))
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
	assert((rtnType == Nav) || (UlamType::compare(rtnType, futi, m_state) == UTIC_SAME));
	rtnType = futi;
	++it;
      }
    return rtnType;
  } //getCustomArrayReturnType

  //similar to
  u32 SymbolFunctionName::getCustomArrayIndexTypeFor(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    std::vector<Node *> argNodes;
    argNodes.push_back(rnode);

    SymbolFunction * fsym = NULL;
    u32 camatches = findMatchingFunctionWithSafeCasts(argNodes, fsym, hasHazyArgs);

    if(camatches == 1)
      {
	Symbol * asym = fsym->getParameterSymbolPtr(0); //1st arg is index
	assert(asym);
	idxuti = asym->getUlamTypeIdx();
      }

    argNodes.clear();
    return camatches;
  } //getCustomArrayIndexTypeFor

  void SymbolFunctionName::linkToParentNodesInFunctionDefs(NodeBlockClass * p)
  {
    NNO pno = p->getNodeNo();
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	assert(fsym->getBlockNoOfST() == pno); //was fnsym nodeno == funcdef block in u.1.1.1
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func); //how would a function symbol be without a body? perhaps an ACCESSOR to-be-made?
	func->updateLineage(pno);
	++it;
      }
    assert(getBlockNoOfST() == pno); // same as template? sfn too
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

  void SymbolFunctionName::countNavNodesInFunctionDefs(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    std::map<std::string, SymbolFunction *>::iterator it = m_mangledFunctionNames.begin();
    u32 countNavs = 0;
    u32 countHzy = 0;
    u32 countUnset = 0;

    while(it != m_mangledFunctionNames.end())
      {
	SymbolFunction * fsym = it->second;
	NodeBlockFunctionDefinition * func = fsym->getFunctionNode();
	assert(func);

	u32 fcntnavs = ncnt;
	u32 fcnthzy = hcnt;
	u32 fcntunset = nocnt;
	//func->countNavHzyNoutiNodes(fcntnavs, fcnthzy, fcntunset);
	func->countNavHzyNoutiNodes(ncnt, hcnt, nocnt);
	if((ncnt - fcntnavs) > 0)
	  {
	    std::string fkey = it->first;
	    std::ostringstream msg;
	    msg << (ncnt - fcntnavs) << " nodes with erroneous types remain in function <";
	    msg << m_state.m_pool.getDataAsString(getId());
	    msg << "> (" << fkey.c_str() << ")";
	    MSG(func->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	    countNavs += (ncnt - fcntnavs);
	  }

	if((hcnt - fcnthzy) > 0)
	  {
	    std::string fkey = it->first;
	    std::ostringstream msg;
	    msg << (hcnt - fcnthzy) << " nodes with unresolved types remain in function <";
	    msg << m_state.m_pool.getDataAsString(getId());
	    msg << "> (" << fkey.c_str() << ")";
	    MSG(func->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	    countHzy += (hcnt - fcnthzy);
	  }

	if((nocnt - fcntunset) > 0)
	  {
	    std::string fkey = it->first;
	    std::ostringstream msg;
	    msg << (nocnt - fcntunset) << " nodes with unset types remain in function <";
	    msg << m_state.m_pool.getDataAsString(getId());
	    msg << "> (" << fkey.c_str() << ")";
	    MSG(func->getNodeLocationAsString().c_str(), msg.str().c_str(), INFO);
	    countUnset += (nocnt - fcntunset);
	  }
	++it;
      }

    //SUMMARIZE:
    if(countNavs > 0)
      {
	u32 numfuncs = m_mangledFunctionNames.size();
	std::ostringstream msg;
	msg << "Summary: " << countNavs << " nodes with erroneous types remain in ";
	if(numfuncs > 1)
	  msg << "all " <<  numfuncs << " functions <";
	else
	  msg << " a single function <";
	msg << m_state.m_pool.getDataAsString(getId()) << "> in class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(m_state.getFullLocationAsString(getLoc()).c_str(), msg.str().c_str(), INFO);
	//ncnt += countNavs;
      }

    if(countHzy > 0)
      {
	u32 numfuncs = m_mangledFunctionNames.size();
	std::ostringstream msg;
	msg << "Summary: " << countHzy << " nodes with unresolved types remain in ";
	if(numfuncs > 1)
	  msg << "all " <<  numfuncs << " functions <";
	else
	  msg << " a single function <";
	msg << m_state.m_pool.getDataAsString(getId()) << "> in class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(m_state.getFullLocationAsString(getLoc()).c_str(), msg.str().c_str(), INFO);
	//hcnt += countHzy;
      }

    if(countUnset > 0)
      {
	u32 numfuncs = m_mangledFunctionNames.size();
	std::ostringstream msg;
	msg << "Summary: " << countUnset << " nodes with unset types remain in ";
	if(numfuncs > 1)
	  msg << "all " <<  numfuncs << " functions <";
	else
	  msg << " a single function <";
	msg << m_state.m_pool.getDataAsString(getId()) << "> in class: ";
	msg << m_state.getUlamTypeNameBriefByIndex(m_state.getCompileThisIdx()).c_str();
	MSG(m_state.getFullLocationAsString(getLoc()).c_str(), msg.str().c_str(), INFO);
	//nocnt += countUnset;
      }
    return;
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
