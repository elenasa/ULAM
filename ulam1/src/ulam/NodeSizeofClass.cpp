#include <stdlib.h>
#include "NodeSizeofClass.h"
#include "CompilerState.h"

namespace MFM {

  NodeSizeofClass::NodeSizeofClass(s32 neguti, CompilerState & state) : NodeTerminal(neguti, state)
  {}

  NodeSizeofClass::NodeSizeofClass(u32 val, CompilerState & state) : NodeTerminal(val, state) {}


  NodeSizeofClass::~NodeSizeofClass(){}


  const std::string NodeSizeofClass::prettyNodeName()
  {
    return nodeName(__PRETTY_FUNCTION__);
  }


  void NodeSizeofClass::constantFold(Token tok)
  {
    assert(0);  //override NodeTerminal
  } //constantFold


  UTI NodeSizeofClass::checkAndLabelType()
  {
    if(m_constant.sval < 0)
      {
	UTI cuti = -m_constant.sval; //out-of-band
	UlamType * cut = m_state.getUlamTypeByIndex(cuti);
	if(cut->getUlamClass() != UC_INCOMPLETE)
	  {
	    m_constant.uval = m_state.getUlamTypeByIndex(cuti)->getBitSize();
	    setNodeType(m_state.getUlamTypeOfConstant(Unsigned)); // an Unsigned constant
	  }
	else
	  {
	    std::ostringstream msg;
	    msg << "Sizeof class: " << m_state.getUlamTypeNameByIndex(cuti).c_str() << " is still incomplete and unknown";
	    MSG(getNodeLocationAsString().c_str(), msg.str().c_str(), ERR);
	  }
      }
    return getNodeType(); //updated to Unsigned, hopefully
  }  //checkandLabelType

} //end MFM
