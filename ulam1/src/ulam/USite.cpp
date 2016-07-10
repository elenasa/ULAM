#include <assert.h>
#include "USite.h"
#include "CompilerState.h"

namespace MFM {

  USite::USite(): m_live(false)
  {
    init();
  }


  USite::~USite()
  {}


  void USite::init()
  {
    m_site = UlamValue::makeAtom(); //type set 'Atom'; clears to zeros
  }


  UTI USite::getElementTypeNumber()
  {
    return m_site.getAtomElementTypeIdx();
  }


  void USite::setElementTypeNumber(UTI type)
  {
    m_site.setAtomElementTypeIdx(type);
  }


  bool USite::isSiteLive()
  {
    return m_live;
  }

  void USite::setSiteLive(bool b)
  {
    m_live = b;
  }


  UlamValue USite::getSiteUlamValue()
  {
    return m_site;
  }

  void USite::setSiteUlamValue(UlamValue uv)
  {
    m_site = uv;
  }

} //MFM
