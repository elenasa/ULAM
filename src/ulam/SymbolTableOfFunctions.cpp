#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTableOfFunctions.h"
#include "SymbolFunctionName.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTableOfFunctions::SymbolTableOfFunctions(CompilerState& state): SymbolTable(state) { }

  SymbolTableOfFunctions::SymbolTableOfFunctions(const SymbolTableOfFunctions& ref) : SymbolTable(ref) { }

  SymbolTableOfFunctions::~SymbolTableOfFunctions() { }

  //called by NodeBlock.
  u32 SymbolTableOfFunctions::getTotalSymbolSize()
  {
    u32 totalsizes = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	u32 depth = ((SymbolFunctionName *) sym)->getDepthSumOfFunctions();
	totalsizes += depth;
	//was used for debugging (see NodeBlockClass eval)
	m_state.abortShouldntGetHere(); //function symbols are not in same table as variables
      }
    it++;
    return totalsizes;
  } //getTotalSymbolSize

  //convert UTI to mangled strings to insure overload uniqueness
  void SymbolTableOfFunctions::checkTableOfFunctions(FSTable& mangledFunctionMap, u32& probcount)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->checkFunctionNames(mangledFunctionMap, probcount);
	  }
	it++;
      }
    return;
  } //checkTableOfFunctions

  void SymbolTableOfFunctions::checkTableOfFunctionsSignatureReturnTypes(FSTable& mangledFunctionMap, u32& probcount)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->checkFunctionSignatureReturnTypes(mangledFunctionMap, probcount);
	  }
	it++;
      }
    return;
  } //checkTableOfFunctionsSignatureReturnTypes

  void SymbolTableOfFunctions::linkToParentNodesAcrossTableOfFunctions(NodeBlockClass * p)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->linkToParentNodesInFunctionDefs(p);
	  }
	it++;
      }
  } //linkToParentNodesAcrossTableOfFunctions

  void SymbolTableOfFunctions::updatePrevBlockPtrAcrossTableOfFunctions(NodeBlockClass * p)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->updatePrevBlockPtrInFunctionDefs(p);
	  }
	it++;
      }
  } //updatePrevBlockPtrAcrossTableOfFunctions

  bool SymbolTableOfFunctions::findNodeNoAcrossTableOfFunctions(NNO n, Node*& foundNode)
  {
    bool rtnfound = false;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    if(((SymbolFunctionName *) sym)->findNodeNoInFunctionDefs(n, foundNode))
	      {
		rtnfound = true;
		break;
	      }
	  }
	it++;
      }
    return rtnfound;
  } //findNodeNoAcrossTableOfFunctions

  void SymbolTableOfFunctions::labelTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->labelFunctions();
	  }
	it++;
      }
  } //labelTableOfFunctions

  void SymbolTableOfFunctions::printUnresolvedLocalVariablesForTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isFunction());
	((SymbolFunctionName *) sym)->printUnresolvedLocalVariablesInFunctionDefs();
	it++;
      }
  } //printUnresolvedVariablesForTableOfClasses

  void SymbolTableOfFunctions::countNavNodesAcrossTableOfFunctions(u32& ncnt, u32& hcnt, u32& nocnt)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->countNavNodesInFunctionDefs(ncnt, hcnt, nocnt);
	  }
	it++;
      }
    return;
  } //countNavNodesAcrossTableOfFunctions

  void SymbolTableOfFunctions::calcMaxDepthForTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	((SymbolFunctionName *) sym)->calcMaxDepthOfFunctions();
	it++;
      }
    return;
  } //calcMaxDepthForTableOfFunctions

  void SymbolTableOfFunctions::calcMaxIndexForVirtualTableOfFunctions(s32& maxidx)
  {
    UTI cuti = m_state.getCompileThisIdx();
    u32 accummaxes = 0;
    bool uninitbases = false;

    BaseclassWalker walker;

    //initialize this classes VTable to super classes' VTable, or empty
    // some entries may be modified; or table may expand
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);

    // get ancestors accumulated max index of originating virtual funcs (entire tree)
    walker.addAncestorsOf(csym);

    UTI baseuti = Nouti;
    while(walker.getNextBase(baseuti, m_state))
      {
	SymbolClass * basecsym = NULL;
	if(m_state.alreadyDefinedSymbolClass(baseuti, basecsym))
	  {
	    s32 mymax = basecsym->getOrigVTableSize(); //originating only

	    if(mymax == UNKNOWNSIZE)
	      uninitbases = true;
	    else
	      accummaxes += mymax;

	    if(uninitbases)
	      {
		maxidx = UNKNOWNSIZE;
		return; //short-circuit
	      }
	    walker.addAncestorsOf(basecsym); // visit all bases
	  }
      } //end while

    if(m_idToSymbolPtr.empty() && (accummaxes == 0))
      {
	assert(maxidx <= 0);
    	maxidx = 0; //use zero when empty
      }
    else
      {
	maxidx = accummaxes;
      }

    csym->initVTable(maxidx);
    //return and use NodeFuncDecl in parse tree order to assign new vowned indexes (ulam-5)
    return;
  } //calcMaxIndexForVirtualTableOfFunctions

  void SymbolTableOfFunctions::checkAbstractInstanceErrorsAcrossTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	((SymbolFunctionName *) sym)->checkAbstractInstanceErrorsInFunctions();
	it++;
      }
    return;
  } //checkAbstractInstanceErrorsAcrossTableOfFunctions

  //called by current Class block on its function ST
  bool SymbolTableOfFunctions::checkCustomArrayTypeFuncs()
  {
    bool rtnBool = false;
    NodeBlockContext * cblock = m_state.getContextBlock();
    assert(cblock && cblock->isAClassBlock());
    NodeBlockClass * currClassBlock = (NodeBlockClass *) cblock;

    Symbol * fnsymget = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsymget))
      {
	u32 probcount = 0;
	//LOOP over SymbolFunctions to verify same return type
	// CANT use UTI directly, must build string of keys to compare
	// as they may change.

	//class type should already be flagged as a custom array
	UTI cuti = currClassBlock->getNodeType();
	if(!m_state.isClassACustomArray(cuti))
	  {
	    std::ostringstream msg;
	    msg << "Custom array get method '";
	    msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str();
	    msg << "' FOUND in class: " << m_state.getUlamTypeByIndex(cuti)->getUlamTypeNameOnly().c_str();
	    MSG(cblock->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	    probcount++;
	  }
	else
	  {
	    UTI caType = Nouti;
	    probcount = ((SymbolFunctionName *) fnsymget)->checkCustomArrayGetFunctions(caType); //sets caType
	  }
	rtnBool = (probcount == 0);
      } //get found
    else
      {
	UTI cuti = m_state.getCompileThisIdx();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(((UlamTypeClass *) cut)->isCustomArray())
	  {
	    std::ostringstream msg;
	    msg << "Custom array get method '";
	    msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str();
	    msg << "' NOT FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	    MSG(cblock->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }

    Symbol * fnsymset = NULL;
    if(isInTable(m_state.getCustomArraySetFunctionNameId(), fnsymset))
      {
	UTI cuti = m_state.getCompileThisIdx();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);

	std::ostringstream msg;
	msg << "Deprecated custom array set method '";
	msg << m_state.m_pool.getDataAsString(m_state.getCustomArraySetFunctionNameId()).c_str();
	msg << "' FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	msg << "; let '";
	msg << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str();
	msg << "' return a reference";
	MSG(cblock->getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	rtnBool = false; //t3919
      }
    return rtnBool;
  } //checkCustomArrayTypeFuncs

  //called by current Class block on its function ST
  UTI SymbolTableOfFunctions::getCustomArrayReturnTypeGetFunction()
  {
    UTI caType = Nouti;
    Symbol * fnsym = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsym))
      {
	caType = ((SymbolFunctionName *) fnsym)->getCustomArrayReturnType();
      }
    return caType;
  } //getCustomArrayReturnTypeGetFunction

  //called by current Class block on its function ST; need to find
  // a "safe" match for rt.
  u32 SymbolTableOfFunctions::getCustomArrayIndexTypeGetFunction(Node * rnode, UTI& idxuti, bool& hasHazyArgs)
  {
    u32 camatches = 0;
    Symbol * fnsym = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsym))
      {
	camatches = ((SymbolFunctionName *) fnsym)->getCustomArrayIndexTypeFor(rnode, idxuti, hasHazyArgs);
      }
    return camatches;
  } //getCustomArrayIndexTypeGetFunction

  //called by current Class block on its function ST;
  bool SymbolTableOfFunctions::hasCustomArrayLengthofFunction()
  {
    Symbol * fnsym = NULL;
    return isInTable(m_state.getCustomArrayLengthofFunctionNameId(), fnsym);
  } //hasCustomArrayLengthofFunction

  u32 SymbolTableOfFunctions::countNativeFuncDeclsForTableOfFunctions()
  {
    u32 nativeCount = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	nativeCount += ((SymbolFunctionName *) sym)->countNativeFuncDecls();
	it++;
      }
    return nativeCount;
  } //countNativeFuncDeclsForTableOfFunctions

} //end MFM
