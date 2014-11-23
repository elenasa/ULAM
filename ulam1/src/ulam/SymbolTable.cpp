#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTable.h"
#include "SymbolFunctionName.h"
#include "SymbolVariable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTable::SymbolTable(CompilerState& state): m_state(state){}
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
	assert( symptrref->getId() == id);  //overkill?

	return true;
      }

    return false;
  }

  void SymbolTable::addToTable(u32 id, Symbol* sptr)
  {
    m_idToSymbolPtr.insert(std::pair<u32,Symbol*> (id,sptr));
  }


  Symbol * SymbolTable::getSymbolPtr(u32 id)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	return it->second;
      }

    return NULL;  //impossible!!
  }

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
  }

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
	//if(sym->isTypedef() || sym->isDataMember())
	if(sym->isTypedef() || (sym->isDataMember() && !sym->isElementParameter()))
	{
	  sym->printPostfixValuesOfVariableDeclarations(fp, slot, startpos, classtype);
	}
	it++;
      }
  }


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
  }


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
  }


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
  }


  void SymbolTable::labelTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	if( ((SymbolClass *) sym)->getUlamClass() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class <"  << m_state.getUlamTypeNameByIndex(sym->getUlamTypeIdx()).c_str() << "> was never defined, fails labeling";
	    MSG("", msg.str().c_str(),ERR);
	    //assert(0); wasn't a class at all, e.g. out-of-scope typedef/variable
	    break;
	  }
	else
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    m_state.m_classBlock = classNode;
	    m_state.m_currentBlock = m_state.m_classBlock;

	    classNode->checkAndLabelType();
	    classNode->setNodeType(sym->getUlamTypeIdx()); //resets class' type (was Void). sweet.
	  }
	it++;
      }
  } //labelTableOfClasses


  // separate pass...after labeling all classes is completed;
  bool SymbolTable::setBitSizeOfTableOfClasses()
  {
    bool aok = true;

    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(sym->isClass());
	if( ((SymbolClass *) sym)->getUlamClass() == UC_INCOMPLETE)
	  {
	    std::ostringstream msg;
	    msg << "Incomplete Class <"  << m_state.getUlamTypeNameByIndex(sym->getUlamTypeIdx()).c_str() << "> was never defined, fails sizing";
	    MSG("", msg.str().c_str(), ERR);
	    //m_state.completeIncompleteClassSymbol(sym->getUlamTypeIdx()); //too late
	    aok = false;  //moved here;
	  }

	//else
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    //if(!classNode) continue; //infinite loop "Incomplete Class <> was never defined, fails sizing"

	    m_state.m_classBlock = classNode;
	    m_state.m_currentBlock = m_state.m_classBlock;

	    s32 totalbits = 0;
	    if(((SymbolClass *) sym)->isQuarkUnion())
	      totalbits = classNode->getMaxBitSizeOfVariableSymbolsInTable(); //data members only
	    else
	      totalbits = classNode->getBitSizesOfVariableSymbolsInTable(); //data members only
#if 0
	    //disabled 11082014
	    if(totalbits == 0)
	      {
		std::ostringstream msg;
		msg << "< zero bit size symbol!! " << m_state.getUlamTypeNameByIndex(sym->getUlamTypeIdx()).c_str() << "(" << totalbits << ")";
		m_state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
		aok = false;
	      }
	    else
#endif
	      {
		UTI sut = sym->getUlamTypeIdx();
		m_state.setBitSize(sut, totalbits);  //"scalar" Class bitsize  KEY ADJUSTED

		//std::ostringstream msg;
		//msg << "symbol size is aok (=" << totalbits << ", total= " << sut->getTotalBitSize() << ") " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
	      }
	  }
	it++;
      }
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

	NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	assert(classNode);
	
	UTI suti = sym->getUlamTypeIdx();
	u32 total = m_state.getTotalBitSize(suti);  
	UlamType * sut = m_state.getUlamTypeByIndex(suti);
	ULAMCLASSTYPE classtype = sut->getUlamClass();
	s32 remaining = (classtype == UC_ELEMENT ? MAXSTATEBITS - total : MAXBITSPERQUARK - total);

	std::ostringstream msg;
	msg << "TotalSize of " << (classtype == UC_ELEMENT ? "element <" : "quark   <") << m_state.m_pool.getDataAsString(sym->getId()).c_str() << ">:\t" << total << "\t(" << remaining << " bits available).";
	//m_state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_INFO);
	std::cerr << msg.str().c_str() << std::endl;

	it++;
      }
  }


  s32 SymbolTable::getTotalVariableSymbolsBitSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    s32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());

	// don't count typedef's or element parameters toward total
	//if(!sym->isTypedef())
	if(!sym->isTypedef() && !sym->isElementParameter())
	  {
	    UTI sut = sym->getUlamTypeIdx();
	    s32 symsize = calcVariableSymbolTypeSize(sut);  //recursively

	    if(symsize == CYCLEFLAG)  // was < 0
	      {
		std::ostringstream msg;
		msg << "cycle error!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
		MSG("", msg.str().c_str(),ERR);
	      }
	    else if(symsize == EMPTYSYMBOLTABLE)
	      {
		symsize = 0;
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
	      }
	    else
	      {
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
		//std::ostringstream msg;
		//msg << "symbol size is " << symsize << " (total = " << sut->getTotalBitSize() << ") " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
	      }
	    totalsizes += m_state.getTotalBitSize(sut);
	  }
	it++;
      }
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
	//if(!sym->isTypedef())
	if(!sym->isTypedef() && !sym->isElementParameter())
	  {
	    UTI sut = sym->getUlamTypeIdx();
	    s32 symsize = calcVariableSymbolTypeSize(sut);  //recursively

	    if(symsize == CYCLEFLAG)  // was < 0
	      {
		std::ostringstream msg;
		msg << "cycle error!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
		MSG("", msg.str().c_str(),ERR);
	      }
	    else if(symsize == EMPTYSYMBOLTABLE)
	      {
		symsize = 0;
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
	      }
	    else
	      {
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
		//std::ostringstream msg;
		//msg << "symbol size is " << symsize << " (total = " << sut->getTotalBitSize() << ") " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
	      }

	    if((s32) m_state.getTotalBitSize(sut) > maxsize)
	      maxsize = m_state.getTotalBitSize(sut);  //includes arrays
	  }
	it++;
      }
    return maxsize;
  } //getMaxVariableSymbolsBitSize


  s32 SymbolTable::calcVariableSymbolTypeSize(UTI argut)
  {
    s32 totbitsize = CYCLEFLAG;
    if(m_state.getUlamTypeByIndex(argut)->getUlamClass() == UC_NOTACLASS)
      {
	totbitsize = m_state.getBitSize(argut);
	assert(totbitsize >= 0);
	return totbitsize;
      }

    //not a primitive
    if(m_state.getArraySize(argut) > 0)
      {
	if((totbitsize = m_state.getBitSize(argut)) > 0)
	  {
	    return totbitsize;
	  }
	if(totbitsize == CYCLEFLAG)  //was < 0
	  {
	    //error! cycle
	    assert(0);
	    return CYCLEFLAG;
	  }
	if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0;  //empty, ok
	  }
	else // totbitsize == 0
	  {
	    m_state.setBitSize(argut, CYCLEFLAG);  //before the recusive call..
	    //get base type
	    SymbolClass * csym = NULL;
	    UlamType * aut = m_state.getUlamTypeByIndex(argut);
	    if(m_state.alreadyDefinedSymbolClass(aut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	      {
		UTI cut = csym->getUlamTypeIdx();
		return calcVariableSymbolTypeSize(cut);  // NEEDS CORRECTION
	      }
	  }
      }
    else  // not primitive type, not array
      {
	if((totbitsize = m_state.getBitSize(argut)) > 0)
	  {
	    return totbitsize;
	  }

	if(totbitsize == CYCLEFLAG) // was < 0
	  {
	    //error! cycle
	    return CYCLEFLAG;
	  }
	else if(totbitsize == EMPTYSYMBOLTABLE)
	  {
	    return 0;  //empty, ok
	  }
	else //totbitsize == 0
	  {
	    //get base type
	    SymbolClass * csym = NULL;
	    UlamType * aut = m_state.getUlamTypeByIndex(argut);
	    if(m_state.alreadyDefinedSymbolClass(aut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	      {
		UTI cut = csym->getUlamTypeIdx();
		s32 csize;
		
		if((csize = m_state.getBitSize(cut)) > 0)
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
		    
		    csize = classblock->getBitSizesOfVariableSymbolsInTable(); //data members only
		    return csize;
		  }
	      }
	  } //totbitsize == 0
      } //not primitive, not array
    return CYCLEFLAG;
  } //calcVariableSymbolTypeSize (recurisvely) 


  void SymbolTable::packBitsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	// quark union keep default pos = 0 for each data member, hence skip packing bits.
	if(!((SymbolClass *) sym)->isQuarkUnion())
	  {
	    ((SymbolClass *) sym)->getClassBlockNode()->packBitsForVariableDataMembers(); 
	  }
	it++;
      }
  }


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
	//if(!sym->isTypedef() && sym->isDataMember())
	//if(!sym->isTypedef() && sym->isDataMember() && !((SymbolClass *) sym)->isQuarkUnion())
	if(!sym->isTypedef() && sym->isDataMember() && !sym->isElementParameter() && !((SymbolClass *) sym)->isQuarkUnion())
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->setPosOffset(offsetIntoAtom);

	    offsetIntoAtom += m_state.getTotalBitSize(sym->getUlamTypeIdx());  // times array size 
	  }
	it++;
      }
  }
#endif

  //bypasses THIS class being compiled
  void SymbolTable::generateIncludesForTableOfClasses(File * fp)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	
	if(sym->getId() != m_state.m_compileThisId)
	  {
	    m_state.indent(fp);
	    fp->write("#include \"");
	    fp->write(m_state.getFileNameForAClassHeader(sym->getId()).c_str());
	    fp->write("\"\n");
	  }
	it++;
      }
    //fp->write("\n");
  }


#if 0
  //not sure we use this; go back and forth between the files that are output
  // if just this class, then NodeProgram can start the ball rolling
  void SymbolTable::genCodeForTableOfClasses(FileManager * fm)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());

	//output header/body for THIS class only
	if(sym.getId() == m_state.m_compileThisId)
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);

	    m_state.m_classBlock = classNode;
	    m_state.m_currentBlock = m_state.m_classBlock;

	    ULAMCLASSTYPE uct = ((SymbolClass *) sym)->getUlamClass();
	    if(uct == UC_ELEMENT)
	      {
		//output both header and body in separate files
		File * fp = fm->fopen(state.getFileNameForAClassHeader(sym.getId()).c_str(),WRITE);
		assert(fp);

		classNode->genCode(fp, uvpass);	
		delete fp;

		// output body for This Class only
		File * fpb = fm->fopen(m_state.getFileNameForThisClassBody().c_str(),WRITE);
		assert(fpb);
		
		classNode->genCodeBody(fpb, uvpass);	
		delete fpb;
	      }
	    else
	      {
		// for quarks output template struct in .h
		assert(uct == UC_QUARK);
		File * fp = fm->fopen(state.getFileNameForAClassHeader(sym.getId()).c_str(),WRITE);
		assert(fp);

		classNode->genCodeBody(fp, uvpass);	
		delete fp;
	      }
	  }
	it++;
      }
  }
#endif


  u32 SymbolTable::getTableSize()
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }


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
  }

} //end MFM
