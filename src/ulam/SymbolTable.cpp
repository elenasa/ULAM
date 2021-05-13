#include <assert.h>
#include <stdio.h>
#include <iostream>
#include "SymbolTable.h"
#include "CompilerState.h"

namespace MFM {

  SymbolTable::SymbolTable(CompilerState& state): m_state(state) { }

  SymbolTable::SymbolTable(const SymbolTable& ref) : m_state(ref.m_state)
  {
    copyATableHere(ref);
  }

  SymbolTable::~SymbolTable()
  {
    //need to delete contents; ownership transferred here!!
    this->clearTheTable();
  }

  void SymbolTable::copyATableHere(const SymbolTable & fmst)
  {
    std::map<u32, Symbol* >::const_iterator it = fmst.m_idToSymbolPtr.begin();
    while(it != fmst.m_idToSymbolPtr.end())
      {
	u32 fid = it->first;
	Symbol * found = it->second;
	Symbol * cloned = found->clone();
	addToTable(fid, cloned);
	it++;
      }
  }

  SymbolTable& SymbolTable::operator=(const SymbolTable& fmst)
  {
    copyATableHere(fmst);
    return *this;
  }

  void SymbolTable::mergeATableHere(const SymbolTable & fmst)
  {
    std::map<u32, Symbol* >::const_iterator it = fmst.m_idToSymbolPtr.begin();
    while(it != fmst.m_idToSymbolPtr.end())
      {
	u32 fid = it->first;
	Symbol * found = it->second;
	Symbol * copyof = NULL;
	if(!isInTable(fid,copyof))
	  {
	    copyof = found->clone();
	    addToTable(fid, copyof);
	  }
	it++;
      }
  }

  void SymbolTable::mergedTableDifferences(const SymbolTable & fmst, SymbolTable & diffst)
  {
    u32 myTableSize = getTableSize();
    u32 fromTableSize = fmst.getTableSize();
    assert(myTableSize > fromTableSize);
    std::map<u32, Symbol* >::const_iterator it = m_idToSymbolPtr.begin();
    while(it != m_idToSymbolPtr.end())
      {
	u32 fid = it->first;
	Symbol * found = it->second;
	Symbol * copyof = NULL;
	if(!fmst.isInTable(fid, copyof))
	  {
	    copyof = found->clone();
	    diffst.addToTable(fid, copyof);
	  }
	//else already here! skip clone
	it++;
      }
    assert(getTableSize() == fmst.getTableSize() + diffst.getTableSize());
  }

  u32 SymbolTable::tableTokenNamesAsAString(std::string& str)
  {
    u32 count = 0;
    std::map<u32, Symbol* >::const_iterator it = m_idToSymbolPtr.begin();
    std::ostringstream msg;
    while(it != m_idToSymbolPtr.end())
      {
	//u32 fid = it->first;
	Symbol * sym = it->second;
	Token * pTok = sym->getTokPtr();
	if(count > 0)
	  msg << ", ";
	msg << m_state.getTokenDataAsString(*pTok).c_str();
	count++;
	it++;
      }
    str = msg.str();
    return count;
  }

  void SymbolTable::clearTheTable()
  {
    std::map<u32, Symbol* >::iterator it = this->m_idToSymbolPtr.begin();
    while(it != this->m_idToSymbolPtr.end())
      {
	Symbol * symptr = it->second;
	delete symptr;
	it->second = NULL;
	it++;
      }
    this->m_idToSymbolPtr.clear();
  } //clearTheTable

  bool SymbolTable::isInTable(u32 id, Symbol * & symptrref)
  {
    if(m_idToSymbolPtr.empty()) return false;
    std::map<u32, Symbol* >::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	symptrref = it->second;
	assert( symptrref->getId() == id);
	return true;
      }
    return false;
  } //isInTable

  bool SymbolTable::isInTable(u32 id, Symbol * & symptrref) const
  {
    if(m_idToSymbolPtr.empty()) return false;
    std::map<u32, Symbol* >::const_iterator it = m_idToSymbolPtr.find(id);
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
    std::pair<std::map<u32, Symbol *>::iterator, bool> reti;
    reti = m_idToSymbolPtr.insert(std::pair<u32,Symbol*> (id,sptr));
    assert(reti.second); //false if already existed, i.e. not added (leak?) t41440???
  }

  void SymbolTable::replaceInTable(u32 oldid, u32 newid, Symbol * s)
  {
    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(oldid);
    if(it != m_idToSymbolPtr.end())
      {
	Symbol * oldsym = it->second;
	assert(oldsym == s);
	m_idToSymbolPtr.erase(it);
      }

    std::map<u32,Symbol*>::iterator nit = m_idToSymbolPtr.find(newid);
    if(nit != m_idToSymbolPtr.end())
      {
	Symbol * newsym = nit->second;
	assert(newsym != s);
	m_idToSymbolPtr.erase(nit);
      }

    addToTable(newid, s);
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
	m_idToSymbolPtr.erase(it); //doesn't delete, up to caller
	rtnok = true;
      }
    return rtnok;
  } //removeFromTable

  Symbol * SymbolTable::getSymbolPtr(u32 id)
  {
    if(m_idToSymbolPtr.empty()) return NULL;

    std::map<u32,Symbol*>::iterator it = m_idToSymbolPtr.find(id);
    if(it != m_idToSymbolPtr.end())
      {
	return it->second;
      }
    return NULL; //impossible!!
  } //getSymbolPtr

  u32 SymbolTable::getTableSize()
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }

  u32 SymbolTable::getTableSize() const
  {
    return (m_idToSymbolPtr.empty() ? 0 : m_idToSymbolPtr.size());
  }

  u32 SymbolTable::getTotalSymbolSize()
  {
    m_state.abortShouldntGetHere();
    return 0;
  }

} //end MFM
