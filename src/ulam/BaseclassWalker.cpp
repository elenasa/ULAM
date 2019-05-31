#include "BaseclassWalker.h"

namespace MFM {

  BaseclassWalker::BaseclassWalker() : m_breadthfirst(true) {}

  BaseclassWalker::BaseclassWalker(bool breadthfirst) : m_breadthfirst(breadthfirst) {}

  BaseclassWalker::~BaseclassWalker()
  {
    m_bases.clear();
    m_seenset.clear();
  }

  void BaseclassWalker::init(UTI cuti)
  {
    assert(m_bases.empty());
    m_bases.push_back(std::pair<UTI,UTI>(cuti,Nouti)); //optional
  }

  bool BaseclassWalker::isDone()
  {
    return m_bases.empty();
  }

  void BaseclassWalker::addAncestorsOf(SymbolClass * csymptr)
  {
    addAncestorPairsOf(csymptr, Nouti);
    return;
  } //addAncestorsOf (helper)

  void BaseclassWalker::addAncestorPairsOf(SymbolClass * csymptr, UTI basehead)
  {
    assert(csymptr);
    u32 basecount = csymptr->getBaseClassCount() + 1; //include super
    for(u32 i = 0; i < basecount; i++)
      {
	if(m_breadthfirst)
	  m_bases.push_back(std::pair<UTI,UTI>(csymptr->getBaseClass(i), basehead)); //extends FIFO queue with next level of base UTIs
	else
	  m_bases.push_front(std::pair<UTI,UTI>(csymptr->getBaseClass(i), basehead)); //extends LIFO stack with next level of base UTIs
      }
  } //addAncestorPairsOf

  bool BaseclassWalker::getNextBase(UTI& nextbase)
  {
    UTI tmpbasehead; //unused
    bool rtnb = getNextBasePair(nextbase, tmpbasehead);
    return rtnb;
  } //getNextBase (helper)

  bool BaseclassWalker::getNextBasePair(UTI& nextbase, UTI& basehead)
  {
    bool rtnb = false;
    UTI baseuti = Nouti;
    UTI headuti = Nouti;
    std::pair<std::set<UTI>::iterator,bool> ret;

    while(!rtnb && !m_bases.empty())
      {
	std::pair<UTI, UTI> base = m_bases.front(); //always from the front, regardless of breadth or depth-first
	baseuti = base.first;
	headuti = base.second;
	m_bases.pop_front();
	ret = m_seenset.insert(baseuti);
	if(ret.second)
	  {
	    rtnb = true; //first-sighting
	    nextbase = baseuti;
	    basehead = headuti;
	  } //else already-seen, try next one..
      } //end while

    if(!rtnb)
      {
	nextbase = Nouti;
	basehead = Nouti;
      }
    return rtnb;
  } //getNextBasePair

} //MFM
