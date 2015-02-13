#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTable.h"
#include "SymbolFunctionName.h"
#include "SymbolVariable.h"
#include "CompilerState.h"
#include "NodeBlockClass.h"
#include "Node.h"

namespace MFM {

  SymbolTable::SymbolTable(CompilerState& state): m_state(state){}
  SymbolTable::SymbolTable(const SymbolTable& ref) : m_state(ref.m_state)
  {
    std::map<u32, Symbol* >::const_iterator it = ref.m_idToSymbolPtr.begin();
    while(it != ref.m_idToSymbolPtr.end())
      {
	u32 fid = it->first;
	Symbol * found = it->second;
	Symbol * cloned = found->clone();
	addToTable(fid, cloned);
	it++;
      }
  }

  SymbolTable::~SymbolTable()
  {
    // need to delete contents; ownership transferred here!!
    for(std::size_t i = 0; i < m_idToSymbolPtr.size(); i++)
      {
	delete m_idToSymbolPtr[i];
      }
    m_idToSymbolPtr.clear();
  }


  bool SymbolTable::isInTable(u32 id, Symbol * & symptrref)
  {
    std::map<u32, Symbol* >::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	symptrref = it->second;
	assert( symptrref->getId() == id);
	return true;
      }
    return false;
  } //isInTable


  void SymbolTable::addToTable(u32 id, Symbol* sptr)
  {
    m_idToSymbolPtr.insert(std::pair<u32,Symbol*> (id,sptr));
  }

  void SymbolTable::replaceInTable(u32 oldid, u32 newid, Symbol * s)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldid);
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsym = it->second;
	assert(oldsym == s);
	m_idToSymbolPtr.erase(it);
	addToTable(newid, s);
      }
    else
      {
	addToTable(newid, s);
      }
  } //replaceInTable

  void SymbolTable::replaceInTable(Symbol * oldsym, Symbol * newsym)
  {
    assert(oldsym->getId() == newsym->getId());
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldsym->getId());
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsymptr = it->second;
	assert(oldsymptr == oldsym);
	it->second = newsym;
	delete(oldsymptr);
      }
  } //replaceInTable (overload)

  bool SymbolTable::removeFromTable(u32 id, Symbol *& rtnsymptr)
  {
    bool rtnok = false;
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	rtnsymptr = it->second;
	m_idToSymbolPtr.erase(it);
	rtnok = true;
      }
    return rtnok;
  } //removeFromTable

  Symbol * SymbolTable::getSymbolPtr(u32 id)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	return it->second;
      }
    return NULL;  //impossible!!
  } //getSymbolPtr


  u32 SymbolTable::getTableSize()
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }


  //called by NodeBlock.
  u32 SymbolTable::getTotalSymbolSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    u32 depth = ((SymbolFunctionName *) sym)->getDepthSumOfFunctions();
	    totalsizes += depth;
	    assert(0); //function symbols are not in same table as variables
	  }
	else
	  {
	    // typedefs don't contribute to the total bit size
	    if(!sym->isTypedef())
	      {
		totalsizes += m_state.slotsNeeded(sym->getUlamTypeIdx());
	      }
	  }
	it++;
      }
    return totalsizes;
  } //getTotalSymbolSize


  s32 SymbolTable::getTotalVariableSymbolsBitSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    s32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());

	UTI suti = sym->getUlamTypeIdx();
	s32 symsize = calcVariableSymbolTypeSize(suti);  //recursively

	if(symsize == CYCLEFLAG)  // was < 0
	  {
	    std::ostringstream msg;
	    msg << "cycle error!!! " << m_state.getUlamTypeNameByIndex(suti).c_str();
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	  }
	else if(symsize == EMPTYSYMBOLTABLE)
	  {
	    symsize = 0;
	    m_state.setBitSize(suti, symsize);  //total bits NOT including arrays
	  }
	else if(symsize <= UNKNOWNSIZE)
	  {
	    std::ostringstream msg;
	    msg << "UNKNOWN !!! " << m_state.getUlamTypeNameByIndex(suti).c_str() << " UTI" << suti << " while compiling class: " << m_state.getUlamTypeNameByIndex(m_state.m_compileThisIdx).c_str();
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
	    if(variableSymbolWithCountableSize(sym))
	      {
		totalsizes = UNKNOWNSIZE;
		break;
	      }
	  }
	else
	  {
	    m_state.setBitSize(suti, symsize);  //symsize does not include arrays
	  }

	// don't count typedef's or element parameters toward total, nor named constants
	if(variableSymbolWithCountableSize(sym))
	  {
	    totalsizes += m_state.getTotalBitSize(suti); //covers up any unknown sizes; includes arrays
	  }
	it++;
      } //while
    return totalsizes;
  } //getTotalVariableSymbolsBitSize


  s32 SymbolTable::getMaxVariableSymbolsBitSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    s32 maxsize = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());

	// don't count typedef's or element parameters toward max
	if(variableSymbolWithCountableSize(sym))
	  {
	    UTI sut = sym->getUlamTypeIdx();
	    s32 symsize = calcVariableSymbolTypeSize(sut);  //recursively

	    if(symsize == CYCLEFLAG)  // was < 0
	      {
		std::ostringstream msg;
		msg << "cycle error!!!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
		MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	      }
	    else if(symsize == EMPTYSYMBOLTABLE)
	      {
		symsize = 0;
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
	      }
	    else
	      {
		m_state.setBitSize(sut, symsize);  //symsize does not include arrays
	      }

	    if((s32) m_state.getTotalBitSize(sut) > maxsize)
	      maxsize = m_state.getTotalBitSize(sut);  //includes arrays
	  }
	it++;
      }
    return maxsize;
  } //getMaxVariableSymbolsBitSize


  //#define OPTIMIZE_PACKED_BITS
#ifdef OPTIMIZE_PACKED_BITS
  // currently, packing is done by Nodes since the order of declaration is available;
  // but in case we may want to optimize the layout someday,
  // we keep this here since all the symbols are available in one place.
  void SymbolTable::packBitsForTableOfVariableDataMembers()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 offsetIntoAtom = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym) && !((SymbolClass *) sym)->isQuarkUnion())
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->setPosOffset(offsetIntoAtom);
	    offsetIntoAtom += m_state.getTotalBitSize(sym->getUlamTypeIdx());  // times array size
	  }
	it++;
      }
  }
#endif


  s32 SymbolTable::findPosOfUlamTypeInTable(UTI utype)
  {
    s32 posfound = -1;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym))
	  {
	    if(sym->getUlamTypeIdx() == utype)
	      {
		posfound = ((SymbolVariable *) sym)->getPosOffset();
		break;
	      }
	  }
	it++;
      }
    return posfound;
  } //findPosOfUlamTypeInTable


  // replaced with parse tree method to preserve order of declaration
  void SymbolTable::genCodeForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(!sym->isTypedef() && sym->isDataMember()) //including element parameters
	  {
	    ((SymbolVariable *) sym)->generateCodedVariableDeclarations(fp, classtype);
	  }
	it++;
      }
  } //genCodeForTableOfVariableDataMembers (unused)


  void SymbolTable::genCodeBuiltInFunctionsOverTableOfVariableDataMember(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
  {
    // 'has' applies to both quarks and elements
    UTI cuti = m_state.m_compileThisIdx;

    if(declOnly)
      {
	m_state.indent(fp);
	fp->write("//helper method not called directly\n");

	m_state.indent(fp);
	fp->write("s32 ");
	fp->write(m_state.getHasMangledFunctionName(cuti));
	fp->write("(const char * namearg) const;\n\n");
	return;
      }

    m_state.indent(fp);
    if(classtype == UC_ELEMENT)
      fp->write("template<class CC>\n");
    else if(classtype == UC_QUARK)
      fp->write("template<class CC, u32 POS>\n");
    else
      assert(0);

    m_state.indent(fp);
    fp->write("s32 ");  //return pos offset, or -1 if not found

    //include the mangled class::
    fp->write(m_state.getUlamTypeByIndex(cuti)->getUlamTypeMangledName().c_str());
    if(classtype == UC_ELEMENT)
      fp->write("<CC>");
    else if(classtype == UC_QUARK)
      fp->write("<CC, POS>");

    fp->write("::");
    fp->write(m_state.getHasMangledFunctionName(cuti));
    fp->write("(const char * namearg) const\n");
    m_state.indent(fp);
    fp->write("{\n");

    m_state.m_currentIndentLevel++;

    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isDataMember() && variableSymbolWithCountableSize(sym))
	  {
	    UTI suti = sym->getUlamTypeIdx();
	    UlamType * sut = m_state.getUlamTypeByIndex(suti);
	    if(sut->getUlamClass() == UC_QUARK)
	      {
		m_state.indent(fp);
		fp->write("if(!strcmp(namearg,\"");
		fp->write(sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureName(&m_state).c_str());
		fp->write("\")) return ");
		fp->write("(");
		fp->write_decimal(((SymbolVariable *) sym)->getPosOffset());
		fp->write(");   //pos offset\n");
	      }
	  }
	it++;
      }
    fp->write("\n");
    m_state.indent(fp);
    fp->write("return ");
    fp->write("(-1);   //not found\n");

    m_state.m_currentIndentLevel--;
    m_state.indent(fp);
    fp->write("}  //has\n\n");
  } //genCodeBuiltInFunctionsOverTableOfVariableDataMember


  // storage for class members persists, so we give up preserving
  // order of declaration that the NodeVarDecl in the parseTree
  // provides, in order to distinguish between an instance's data
  // members on the STACK verses the classes' data members in
  // EVENTWINDOW.
  void SymbolTable::printPostfixValuesForTableOfVariableDataMembers(File * fp, s32 slot, u32 startpos, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isTypedef() || sym->isConstant() || (sym->isDataMember() && !sym->isElementParameter()))
	{
	  sym->printPostfixValuesOfVariableDeclarations(fp, slot, startpos, classtype);
	}
	it++;
      }
  } //printPostfixValuesForTableOfVariableDataMembers


  // convert UTI to mangled strings to insure overload uniqueness
  void SymbolTable::checkTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->checkFunctionNames();
	  }
	it++;
      }
  } //checkTableOfFunctions

  void SymbolTable::linkToParentNodesAcrossTableOfFunctions(NodeBlockClass * p)
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

  bool SymbolTable::findNodeNoAcrossTableOfFunctions(NNO n, Node*& foundNode)
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
  }//findNodeNoAcrossTableOfFunctions

  void SymbolTable::labelTableOfFunctions()
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


  u32 SymbolTable::countNavNodesAcrossTableOfFunctions()
  {
    u32 totalNavCount = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	if(sym->isFunction())
	  {
	    totalNavCount += ((SymbolFunctionName *) sym)->countNavNodesInFunctionDefs();
	  }
	it++;
      }
    return totalNavCount;
  } //countNavNodesAcrossTableOfFunctions


  //called by current Class block on its function ST
  bool SymbolTable::checkCustomArrayTypeFuncs()
  {
    bool rtnBool = false;
    Symbol * fnsym = NULL;
    if(isInTable(m_state.getCustomArrayGetFunctionNameId(), fnsym))
      {
	//LOOP over SymbolFunctions to get return type, and check
	// if corresponding aset exists and params match.
	// CANT use UTI directly, must build string of keys to compare
	// as they may change.

	// set class type to custom array; the current class block
	// node type was set to its class symbol type at start of parsing it.
	UTI cuti = m_state.m_classBlock->getNodeType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	assert(((UlamTypeClass *) cut)->isCustomArray());
	{
	  std::ostringstream msg;
	  msg << "Custom array get method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str() << "' FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	  MSG("", msg.str().c_str(), DEBUG);
	}

	u32 probcount = ((SymbolFunctionName *) fnsym)->checkCustomArrayFunctions(*this);
	rtnBool = (probcount == 0);
      } //get found
    else
      {
	UTI cuti = m_state.m_compileThisIdx;
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);

	std::ostringstream msg;
	msg << "Custom array get method: '" << m_state.m_pool.getDataAsString(m_state.getCustomArrayGetFunctionNameId()).c_str() << "' NOT FOUND in class: " << cut->getUlamTypeNameOnly().c_str();
	MSG("", msg.str().c_str(), DEBUG);
      }
    return rtnBool;
  } //checkForAndInitializeClassCustomArrayType

  u32 SymbolTable::countNativeFuncDeclsForTableOfFunctions()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 nativeCount = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isFunction());
	nativeCount += ((SymbolFunctionName *) sym)->countNativeFuncDecls();
	it++;
      }
    return nativeCount;
  } //countNativeFuncDeclsForTableOfFunctions


  void SymbolTable::genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype)
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


  void SymbolTable::testForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    m_state.m_err.clearCounts();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	((SymbolClassName *) sym)->testForClassInstances(fp);
	it++;
      } //while

    // output informational warning and error counts
    u32 warns = m_state.m_err.getWarningCount();
    if(warns > 0)
      {
	std::ostringstream msg;
	msg << warns << " warning" << (warns > 1 ? "s " : " ") << "during eval";
	MSG("", msg.str().c_str(), INFO);
      }

    u32 errs = m_state.m_err.getErrorCount();
    if(errs > 0)
      {
	std::ostringstream msg;
	msg << errs << " TOO MANY EVAL ERRORS";
	MSG("", msg.str().c_str(), INFO);
      }
  } //testForTableOfClasses


  void SymbolTable::printPostfixForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	classNode->printPostfix(fp);
	it++;
      } //while
  } //printPostfixForTableOfClasses


  void SymbolTable::printForDebugForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	classNode->print(fp);
	it++;
      } //while
  } //printForDebugForTableOfClasses

  bool SymbolTable::statusUnknownConstantExpressionsInTableOfClasses()
  {
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	aok &= ((SymbolClassName *) sym)->statusUnknownConstantExpressionsInClassInstances();
	it++;
      } //while
    return aok;
  } //statusUnknownConstantExpressionsInTableOfClasses()

  bool SymbolTable::statusNonreadyClassArgumentsInTableOfClasses()
  {
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	aok &= ((SymbolClassName *) sym)->statusNonreadyClassArgumentsInShallowClassInstances();
	it++;
      } //while
    return aok;
  }//statusNonreadyClassArgumentsInTableOfClasses

  bool SymbolTable::cloneInstancesInTableOfClasses()
  {
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());
	aok &= ((SymbolClassName *) sym)->cloneInstances();
	it++;
      } //while
    return aok;
  } //cloneInstancesInTableOfClasses

#if 0
  // done after cloning and before checkandlabel;
  // blocks without prevblocks set, are linked to prev block;
  // used for searching for missing symbols in STs during c&l.
  void SymbolTable::updateLineageForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	((SymbolClassName *) sym)->updateLineageOfClassInstances();
	it++;
      } //while
  } //updateLineageForTableOfClasses
#endif

  void SymbolTable::checkCustomArraysForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	m_state.m_classBlock = classNode;
	m_state.m_currentBlock = m_state.m_classBlock;

	// custom array flag set at parse time
	UTI cuti = classNode->getNodeType();
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(((UlamTypeClass *) cut)->isCustomArray())
	  {
	    classNode->checkCustomArrayTypeFunctions();
	  }
	it++;
      }
  } //checkCustomArraysForTableOfClasses()



  bool SymbolTable::labelTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	SymbolClassName * cnsym = (SymbolClassName *) (it->second);
	assert(cnsym->isClass());
	if( ((SymbolClass *) cnsym)->getUlamClass() == UC_UNSEEN)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class: "  << m_state.getUlamTypeNameByIndex(cnsym->getUlamTypeIdx()).c_str() << " was never defined, fails labeling";
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	    //assert(0); wasn't a class at all, e.g. out-of-scope typedef/variable
	    break;
	  }
	else
	  {
	    cnsym->checkAndLabelClassInstances();
	  }
	it++;
      }
    return (m_state.m_err.getErrorCount() == 0);
  } //labelTableOfClasses

  u32 SymbolTable::countNavNodesAcrossTableOfClasses()
  {
    u32 navcount = 0;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	navcount += ((SymbolClassName *) sym)->countNavNodesInClassInstances();
	it++;
      }
    return navcount;
  } //countNavNodesAcrossTableOfClasses

  // separate pass...after labeling all classes is completed;
  // purpose is to set the size of all the classes, by totalling the size
  // of their data members; returns true if all class sizes complete.
  bool SymbolTable::setBitSizeOfTableOfClasses()
  {
    std::vector<u32> lostClassesIds;
    bool aok = true;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	if( ((SymbolClass *) sym)->getUlamClass() == UC_UNSEEN)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class: "  << m_state.getUlamTypeNameByIndex(sym->getUlamTypeIdx()).c_str() << " was never defined, fails sizing";
	    MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),ERR);
	    //m_state.completeIncompleteClassSymbol(sym->getUlamTypeIdx()); //too late
	    aok = false;  //moved here;
	  }

	// try..
	aok = ((SymbolClassName *) sym)->setBitSizeOfClassInstances();

	//track classes that fail to be sized.
	if(!aok)
	  lostClassesIds.push_back(sym->getId());

	aok = true; //reset for next class
	it++;
      } //next class

    aok = lostClassesIds.empty();
    if(!aok)
      {
	std::ostringstream msg;
	msg << lostClassesIds.size() << " Classes without sizes";
	while(!lostClassesIds.empty())
	  {
	    u32 cid = lostClassesIds.back();
	    msg << ", " << m_state.m_pool.getDataAsString(cid).c_str();
	    lostClassesIds.pop_back();
	  }
	MSG(m_state.getFullLocationAsString(m_state.m_locOfNextLineText).c_str(), msg.str().c_str(),DEBUG);
      }
    else
      {
	// misleading..
	//	std::ostringstream msg;
	//msg << m_idToSymbolPtr.size() << " Class" <<( m_idToSymbolPtr.size() > 1 ? "es ALL " : " ") << "sized SUCCESSFULLY";
	//MSG("", msg.str().c_str(),DEBUG);
      }
    lostClassesIds.clear();
    return aok;
  } //setBitSizeOfTableOfClasses

  // separate pass...after labeling all classes is completed;
  void SymbolTable::printBitSizeOfTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	((SymbolClassName *) sym)->printBitSizeOfClassInstances();
	it++;
      }
  } //printBitSizeOfTableOfClasses

  void SymbolTable::packBitsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym && sym->isClass());

	// quark union keep default pos = 0 for each data member, hence skip packing bits.
	if(!((SymbolClass *) sym)->isQuarkUnion())
	  {
	    ((SymbolClassName *) sym)->packBitsForClassInstances();
	  }
	it++;
      }
  } //packBitsForTableOfClasses

  //bypasses THIS class being compiled
  void SymbolTable::generateIncludesForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	((SymbolClassName *) sym)->generateIncludesForClassInstances(fp);
	it++;
      }
  } //generateIncludesForTableOfClasses

  //bypasses THIS class being compiled
  void SymbolTable::generateForwardDefsForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	((SymbolClassName *) sym)->generateForwardDefsForClassInstances(fp);
	it++;
      }
  } //generateForwardDefsForTableOfClasses

  enum { NORUNTEST = 0, RUNTEST = 1  };

  //test for the current compileThisIdx, with test method
  void SymbolTable::generateTestInstancesForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	//first output all the element typedefs, skipping quarks
	if(((SymbolClass * ) sym)->getUlamClass() != UC_QUARK)
	  ((SymbolClassName *) sym)->generateTestInstanceForClassInstances(fp, NORUNTEST);
	it++;
      } //while for typedefs only

    it = m_idToSymbolPtr.begin();
    s32 idcounter = 1;
    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	//next output all the element typedefs that are m_compileThisId; skipping quarks
	if(sym->getId() == m_state.m_compileThisId && ((SymbolClass * ) sym)->getUlamClass() != UC_QUARK)
	  ((SymbolClassName *) sym)->generateTestInstanceForClassInstances(fp, RUNTEST);
	it++;
	idcounter++;
      } //while to run this test
    fp->write("\n");
  } //generateTestInstancesForTableOfClasses

  void SymbolTable::genCodeForTableOfClasses(FileManager * fm)
  {
    //    mergeInstancesBeforeCodeGenForTableOfClasses();
    UTI saveCompileThisIdx = m_state.m_compileThisIdx;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());

	//output header/body for this class next
	m_state.setCompileThisIdx(sym->getUlamTypeIdx());
	((SymbolClassName *) sym)->generateCodeForClassInstances(fm);
	it++;
      } //while

    m_state.setCompileThisIdx(saveCompileThisIdx);  //restore
  } //genCodeForTableOfClasses

#if 0
  void SymbolTable::mergeInstancesBeforeCodeGenForTableOfClasses()
  {
    UTI saveCompileThisIdx = m_state.m_compileThisIdx;
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	m_state.setCompileThisIdx(sym->getUlamTypeIdx());
	((SymbolClassName *) sym)->mergeClassInstancesBeforeCodeGen();
	it++;
      } //while

    setCtate(compileThisIdx = saveCompileThisIdx);  //restore
  } //mergeInstancesBeforeCodeGenForTableOfClasses
#endif

  // PRIVATE HELPER METHODS:
  s32 SymbolTable::calcVariableSymbolTypeSize(UTI argut)
  {
    if(!m_state.isComplete(argut))
       m_state.constantFoldIncompleteUTI(argut);

    s32 totbitsize = m_state.getBitSize(argut);

    if(m_state.getUlamTypeByIndex(argut)->getUlamClass() == UC_NOTACLASS) //includes Atom type
      {
	return totbitsize;  //arrays handled by caller, just bits here
      }

    //not a primitive (class), array
    if(m_state.getArraySize(argut) > 0)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize;
	  }
	if(totbitsize == CYCLEFLAG)  //was < 0
	  {
	    assert(0);
	    return CYCLEFLAG;
	  }
	if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0;  //empty, ok
	  }
	else
	  {
	    assert(totbitsize <= UNKNOWNSIZE || m_state.getArraySize(argut) == UNKNOWNSIZE);

	    m_state.setBitSize(argut, CYCLEFLAG);  //before the recusive call..

	    //get base type, scalar type of class
	    SymbolClass * csym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(argut, csym))
	      {
		return calcVariableSymbolTypeSize(csym->getUlamTypeIdx());  // NEEDS CORRECTION
	      }
	  }
      }
    else  // not primitive type (class), and not array (scalar)
      {
	if(totbitsize >= 0)
	  {
	    return totbitsize;
	  }

	if(totbitsize == CYCLEFLAG) // was < 0
	  {
	    return CYCLEFLAG;       //error! cycle
	  }
	else if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0;  //empty, ok
	  }
	else
	  {
	    assert(totbitsize == UNKNOWNSIZE);
	    //get base type
	    SymbolClass * csym = NULL;
	    if(m_state.alreadyDefinedSymbolClass(argut, csym))
	      {
		s32 csize;
		if((csize = m_state.getBitSize(csym->getUlamTypeIdx())) >= 0)
		  {
		    return csize;
		  }
		else if(csize == CYCLEFLAG)  // was < 0
		  {
		    //error! cycle..replace with message
		    return csize;
		  }
		else if(csize == EMPTYSYMBOLTABLE)
		  {
		    return 0;  //empty, ok
		  }
		else
		  {
		    // ==0, redo variable total
		    NodeBlockClass * classblock = csym->getClassBlockNode();
		    assert(classblock);

		    //quark cannot contain a copy of itself!
		    assert(classblock != m_state.m_classBlock);

		    if(csym->isQuarkUnion())
		      csize = classblock->getMaxBitSizeOfVariableSymbolsInTable();
		    else
		      csize = classblock->getBitSizesOfVariableSymbolsInTable(); //data members only
		    return csize;
		  }
	      }
	  } //totbitsize == 0
      } //not primitive, not array
    return CYCLEFLAG;
  } //calcVariableSymbolTypeSize (recursively)

  bool SymbolTable::variableSymbolWithCountableSize(Symbol * sym)
  {
    return (!sym->isTypedef() && !sym->isElementParameter() && !sym->isConstant());
  }

} //end MFM
