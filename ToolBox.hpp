#ifndef TOOLBOX_HPP
#define TOOLBOX_HPP

#include "Tool.hpp"
#include "ContourFeature.hpp"

class ToolBox {

public:
  //Default constructor (empty vector)
  ToolBox();

  //Returns a pointer on the most similar tool in toolbox
  Tool* findTheGoodOne(Tool&);

  //Returns a pointer on the last inserted tool
  Tool* last() const;

  //True if toolbox is empty
  bool empty() const;
  
  //Adds tool in toolbox
  void add(Tool& tool);

  //Erase the less used tools
  void clean();

  //Returns toolbox size
  int size() const;
  
  std::vector<Tool> tools;
protected:
  
};

#endif
