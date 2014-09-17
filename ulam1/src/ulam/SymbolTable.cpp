#include <assert.h>
#include <stdio.h>
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
	if(!sym->isTypedef() && sym->isDataMember())
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
  void SymbolTable::printPostfixValuesForTableOfVariableDataMembers(File * fp, ULAMCLASSTYPE classtype)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(sym->isTypedef() || sym->isDataMember())
	{
	  sym->printPostfixValuesOfVariableDeclarations(fp, classtype);
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
  }


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
	  }

	//else
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    //if(!classNode) continue; //infinite loop "Incomplete Class <> was never defined, fails sizing"

	    m_state.m_classBlock = classNode;
	    m_state.m_currentBlock = m_state.m_classBlock;

	    u32 totalbits = classNode->getBitSizesOfVariableSymbolsInTable(); //data members only
	    if(totalbits == 0)
	      {
		//std::ostringstream msg;
		//msg << "setting zero bit size symbol!! " << sym->getUlamType()->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
		aok = false;
	      }
	    else
	      {
		UTI sut = sym->getUlamTypeIdx();
		m_state.setBitSize(sut, totalbits);  //"scalar" Class bitsize  XXXXX ADJUST KEY


		//std::ostringstream msg;
		//msg << "symbol size is aok (=" << totalbits << ", total= " << sut->getTotalBitSize() << ") " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
	      }
	  }
	it++;
      }
    return aok;
  }


  u32 SymbolTable::getTotalVariableSymbolsBitSize()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();
    u32 totalsizes = 0;

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;
	assert(!sym->isFunction());
	if(!sym->isTypedef())
	  {
	    UTI sut = sym->getUlamTypeIdx();
	    s32 symsize = calcVariableSymbolTypeSize(sut);

	    if(symsize < 0)
	      {
		std::ostringstream msg;
		msg << "cycle error!! " << m_state.getUlamTypeNameByIndex(sut).c_str();
		MSG("", msg.str().c_str(),ERR);
	      }
	    else
	      {
		m_state.setBitSize(sut, symsize);  //total bits NOT including arrays
		//std::ostringstream msg;
		//msg << "symbol size is " << symsize << " (total = " << sut->getTotalBitSize() << ") " << sut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureAsString(&state).c_str();
		//state.m_err.buildMessage("", msg.str().c_str(),__FILE__, __func__, __LINE__, MSG_DEBUG);
	      }
	    totalsizes += m_state.getUlamTypeByIndex(sut)->getTotalBitSize();
	  }
	it++;
      }
    return totalsizes;
  }


  s32 SymbolTable::calcVariableSymbolTypeSize(UTI argut)
  {
    s32 totbitsize = -1;
    if(m_state.getUlamTypeByIndex(argut)->getUlamClass() == UC_NOTACLASS)
      {
	totbitsize = m_state.getBitSize(argut);
	assert(totbitsize);
	return totbitsize;
      }

    //not a primitive
    if(m_state.getArraySize(argut) > 0)
      {
	if((totbitsize = m_state.getBitSize(argut)) > 0)
	  {
	    return totbitsize;
	  }
	if(totbitsize < 0)
	  {
	    //error! cycle
	    return -1;
	    assert(0);
	  }
	else // totbitsize == 0
	  {
	    m_state.setBitSize(argut, -1);  //before the recusive call.. HMMMM???
	    //get base type
	    SymbolClass * csym = NULL;
	    UlamType * aut = m_state.getUlamTypeByIndex(argut);
	    if(m_state.alreadyDefinedSymbolClass(aut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	      {
		UTI but = csym->getUlamTypeIdx();
		return calcVariableSymbolTypeSize(but);  // NEEDS CORRECTION
	      }
	  }
      }
    else  // not primitive type, not array
      {
	if((totbitsize = m_state.getBitSize(argut)) > 0)
	  {
	    return totbitsize;
	  }

	if(totbitsize < 0)
	  {
	    //error! cycle
	    return -1;
	  }

	else //totbitsize == 0
	  {
	    //get base type
	    SymbolClass * csym = NULL;
	    UlamType * aut = m_state.getUlamTypeByIndex(argut);
	    if(m_state.alreadyDefinedSymbolClass(aut->getUlamKeyTypeSignature().getUlamKeyTypeSignatureNameId(), csym))
	      {
		UTI but = csym->getUlamTypeIdx();
		s32 bsize;
		
		if((bsize = m_state.getBitSize(but)) > 0)
		  {
		    return bsize;
		  }
		else if(bsize < 0)
		  {
		    //error! cycle..replace with message
		    return bsize;
		  }
		else
		  {
		    // ==0, redo variable total
		    NodeBlockClass * classblock = csym->getClassBlockNode();
		    assert(classblock);
		    assert(classblock != m_state.m_classBlock);
		    
		    bsize = classblock->getBitSizesOfVariableSymbolsInTable(); //data members only
		    return bsize;
		  }
	      }
	  } //totbitsize == 0
      } //not primitive, not array
    return -1;
  } 


  void SymbolTable::packBitsForTableOfClasses()
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	((SymbolClass *) sym)->getClassBlockNode()->packBitsForVariableDataMembers();	
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
	if(!sym->isTypedef() && sym->isDataMember())
	  {
	    //updates the offset with the bit size of sym
	    ((SymbolVariable *) sym)->setPosOffset(offsetIntoAtom);
	    offsetIntoAtom += sym->getUlamType()->getTotalBitSize();  // times array size 
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

		classNode->genCode(fp);	
		delete fp;

		// output body for This Class only
		File * fpb = fm->fopen(m_state.getFileNameForThisClassBody().c_str(),WRITE);
		assert(fpb);
		
		classNode->genCodeBody(fpb);	
		delete fpb;
	      }
	    else
	      {
		// for quarks output template struct in .h
		assert(uct == UC_QUARK);
		File * fp = fm->fopen(state.getFileNameForAClassHeader(sym.getId()).c_str(),WRITE);
		assert(fp);

		classNode->genCodeBody(fp);	
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
		u32 arraysize = m_state.getArraySize(sym->getUlamTypeIdx());
		if(WritePacked(((SymbolVariable *) sym)->isPacked()))
		  totalsizes += 1;
		else	      
		  totalsizes += (arraysize > 0 ? arraysize : 1);
	      }
	  }
	it++;
      }
    return totalsizes;
  }

} //end MFM
