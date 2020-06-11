#include "BaseclassWalker.h"
#include "CompilerState.h"

namespace MFM {

  //BaseclassWalker::BaseclassWalker() : m_breadthfirst(true) {}  //breadth-first as default

  BaseclassWalker::BaseclassWalker() : m_breadthfirst(false) {}   //depth-first as default

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
    if(m_breadthfirst)
      {
	//extends FIFO queue with next level of base UTIs
	for(u32 i = 0; i < basecount; i++)
	  m_bases.push_back(std::pair<UTI,UTI>(csymptr->getBaseClass(i), basehead));
      }
    else
      {
	//extends LIFO stack with next level of base UTIs; reverse order.
	for(s32 i = basecount - 1; i >= 0; i--)
	  m_bases.push_front(std::pair<UTI,UTI>(csymptr->getBaseClass(i), basehead));
      }
  } //addAncestorPairsOf

  bool BaseclassWalker::getNextBase(UTI& nextbase, CompilerState& state)
  {
    UTI tmpbasehead; //unused
    bool rtnb = getNextBasePair(nextbase, tmpbasehead, state);
    return rtnb;
  } //getNextBase (helper)

  bool BaseclassWalker::getNextBasePair(UTI& nextbase, UTI& basehead, CompilerState& state)
  {
    bool rtnb = false;
    UTI baseuti = Nouti;
    UTI headuti = Nouti;
    std::pair<std::set<UTI>::iterator,bool> ret;

    while(!rtnb && !m_bases.empty())
      {
	//always from the front, regardless of breadth or depth-first
	std::pair<UTI, UTI> base = m_bases.front();
	baseuti = base.first;
	headuti = base.second;
	m_bases.pop_front();

	UTI rootuti = baseuti;
	if(!state.isARootUTI(baseuti))
	  {
	    AssertBool gotroot = state.findaUTIAlias(baseuti, rootuti); //t41384
	    assert(gotroot);
	  }

	ret = m_seenset.insert(rootuti);
	if(ret.second)
	  {
	    rtnb = true; //first-sighting
	    nextbase = rootuti;
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
