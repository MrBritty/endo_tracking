#include "ToolBox.hpp"

#define NB_MIN_SELECTIONS 30

ToolBox::ToolBox() {}

Tool* ToolBox::findTheGoodOne(Tool& t) {
  Tool* tool = NULL;
  
  if(tools.empty()){
    return NULL;
  }
  
  
  double currentConfidence = t.compare(tools[0].getFeatures());;
  int theGoodOneIndex = -1;
  double c = 0;
  
  for(unsigned int i=1; i<tools.size(); i++) 
  {
    c = t.compare(tools[i].getFeatures());
    if(c >= currentConfidence && c >= 10) 
	{
      theGoodOneIndex = i;
      currentConfidence = c;
    }
  }

  if(theGoodOneIndex != -1){
    tool = (Tool*)(&tools[theGoodOneIndex]);
    tools[theGoodOneIndex].hasBeenSelected();
  }
  return tool;
}

Tool* ToolBox::last() const {
  if(tools.size() == 0){
    return NULL;
  }
  return  (Tool*)&tools.back();
}

bool ToolBox::empty() const {
  return tools.empty();
}

void ToolBox::add(Tool& tool) {
  tools.push_back(tool);
  tool.setID(tools.size());
}


void ToolBox::clean() {
  for(std::vector<Tool>::iterator it = tools.begin();
      it != tools.end();
      it++) {
    if(it->getSelections() < NB_MIN_SELECTIONS) {
      tools.erase(it);
    }
  }
}

int ToolBox::size() const 
{
  return tools.size();
}
