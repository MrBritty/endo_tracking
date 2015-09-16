#ifndef STATEMACHINE_HPP
#define STATEMACHINE_HPP

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <iostream>
using namespace cv;
using namespace std;


class State_Machine
{
public:
	State_Machine();
	int getTool();
	int getRing(int index);
	vector<int> getState();
	vector<int> getPreviousState();
	void setTool(int val);
	void setRing(int index, int val);
	void setAllRings(int val);
	void update_state(vector<int> new_state);
	string getStatus();
	void setStatus(string new_status);
	void setPreviousState(vector<int> state);

protected:
	
	int tool;
	string status;
	vector<int> previous_State;
	vector<int> rings;
};

#endif