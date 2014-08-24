#include <assert.h>
#include <stdio.h>
#include "SymbolTable.h"
#include "SymbolFunctionName.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTable::SymbolTable(){}
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


  void SymbolTable::genCodeForTableOfFunctions(File * fp, bool declOnly, ULAMCLASSTYPE classtype, CompilerState * state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	if(sym->isFunction())
	  {
	    ((SymbolFunctionName *) sym)->generateCodedFunctions(fp, declOnly, classtype, state);
	  }
	it++;
      }
  }


  void SymbolTable::labelTableOfClasses(CompilerState& state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	if( ((SymbolClass *) sym)->getUlamClassType() == UC_INCOMPLETE)
	  {
	    assert(0);  //replace with an error message.
	    //error! never found..or defined afterwards.
	  }
	else
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);
	    state.m_classBlock = classNode;
	    classNode->checkAndLabelType();
	    classNode->setNodeType(sym->getUlamType()); //resets class' type (was Void). sweet.

	    u32 totalbits = classNode->getBitSizesOfVariableSymbolsInTable(); //data members only
	    sym->getUlamType()->setBitSize(totalbits);
	  }
	it++;
      }
  }


  //bypasses THIS class being compiled
  void SymbolTable::generateIncludesForTableOfClasses(File * fp, CompilerState& state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());
	
	if(sym->getId() != state.m_compileThisId)
	  {
	    state.indent(fp);
	    fp->write("#include \"");
	    fp->write(state.getFileNameForAClassHeader(sym->getId()).c_str());
	    fp->write("\"\n");
	  }
	it++;
      }
    //fp->write("\n");
  }

#if 0
  //not sure we use this; go back and forth between the files that are output
  // if just this class, then NodeProgram can start the ball rolling
  void SymbolTable::genCodeForTableOfClasses(FileManager * fm, CompilerState& state)
  {
    std::map<u32, Symbol *>::iterator it = m_idToSymbolPtr.begin();

    while(it != m_idToSymbolPtr.end())
      {
	Symbol * sym = it->second;  
	assert(sym->isClass());

	//output header/body for THIS class only
	if(sym.getId() == state.m_compileThisId)
	  {
	    NodeBlockClass * classNode = ((SymbolClass *) sym)->getClassBlockNode();
	    assert(classNode);

	    m_state.m_classBlock = classNode;

	    ULAMCLASSTYPE uct = ((SymbolClass *) sym)->getUlamClassType();
	    if(uct == UC_ELEMENT)
	      {
		//output both header and body in separate files
		File * fp = fm->fopen(state.getFileNameForAClassHeader(sym.getId()).c_str(),WRITE);
		assert(fp);

		classNode->genCode(fp);	
		delete fp;

		// output body for This Class only
		File * fpb = fm->fopen(state.getFileNameForThisClassBody().c_str(),WRITE);
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
	  }
	else
	  {
	    u32 arraysize = sym->getUlamType()->getArraySize();
	    totalsizes += (arraysize > 0 ? arraysize : 1);
	  }
	it++;
      }
    return totalsizes;
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
	    u32 arraysize = sym->getUlamType()->getArraySize();
	    u32 bitsize = sym->getUlamType()->getUlamKeyTypeSignature().getUlamKeyTypeSignatureBitSize();
	    totalsizes += ((arraysize > 0 ? arraysize : 1) * bitsize);
	  }
	it++;
      }
    return totalsizes;
  }

} //end MFM
