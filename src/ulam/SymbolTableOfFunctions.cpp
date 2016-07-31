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
	assert(0); //function symbols are not in same table as variables
      }
    it++;
    return totalsizes;
  } //getTotalSymbolSize

  void SymbolTableOfFunctions::addClassMemberFunctionDescriptionsToMap(UTI classType, ClassMemberMap& classmembers)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());

	((SymbolFunctionName *) sym)->addFunctionDescriptionsToClassMemberMap(classType, classmembers);
	it++;
      }
  } //addClassMemberFunctionDescriptionsToMap

  //convert UTI to mangled strings to insure overload uniqueness
  void SymbolTableOfFunctions::checkTableOfFunctions(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount)
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

  void SymbolTableOfFunctions::checkTableOfFunctionsInAncestor(std::map<std::string, UTI>& mangledFunctionMap, u32& probcount)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->checkFunctionNamesInAncestor(mangledFunctionMap, probcount);
	  }
	it++;
      }
    return;
  } //checkTableOfFunctionsInAncestor

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
    UTI superuti = m_state.isClassASubclass(cuti);

    if(m_idToSymbolPtr.empty() && (superuti == Nouti))
      {
	assert(maxidx <= 0);
    	maxidx = 0; //use zero when empty
      }

    //initialize this classes VTable to super classes' VTable, or empty
    // some entries may be modified; or table may expand
    SymbolClass * csym = NULL;
    AssertBool isDefined = m_state.alreadyDefinedSymbolClass(cuti, csym);
    assert(isDefined);
    csym->initVTable(maxidx);

    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	((SymbolFunctionName *) sym)->calcMaxIndexOfVirtualFunctions(maxidx);
	it++;
      }
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

	    if(!probcount)
	      {
		// for each aset that exists: it has two params, the 2nd is
		// the same as the get return type, and the set return type is Void.
		Symbol * fnsymset = NULL;
		if(isInTable(m_state.getCustomArraySetFunctionNameId(), fnsymset))
		  {
		    probcount = ((SymbolFunctionName *) fnsymset)->checkCustomArraySetFunctions(caType);
		  }
	      }
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

  void SymbolTableOfFunctions::genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->generateCodedFunctions(fp, declOnly, classtype);
	  }
	it++;
      }
  } //genCodeForTableOfFunctions


} //end MFM
