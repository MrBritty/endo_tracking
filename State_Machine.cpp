#include "State_Machine.h"


State_Machine::State_Machine() : tool(-1), rings(6,-1), status("start"), previous_State(6,-1)
{			   

}

int State_Machine:: getTool()
{
	return this ->tool;
}

int State_Machine::getRing (int index)
{
	return this ->rings [index];
}




void State_Machine :: setTool(int val)
{
	this ->tool = val;
}

void State_Machine ::setRing (int index, int val)
{
	this->rings[index] = val;
}


void State_Machine :: setAllRings(int val)
{
	rings.assign(6,val);
}


vector<int> State_Machine :: getState()
{
	return this->rings;
	
}


vector<int> State_Machine :: getPreviousState()
{
	return this ->previous_State;
}


void State_Machine :: setPreviousState(vector<int> state)
{
	this-> previous_State = state;
}

string State_Machine :: getStatus()
{
	return this -> status;
}


void State_Machine ::setStatus(string new_status)
{
	this -> status = new_status;
}

void State_Machine :: update_state(vector<int> new_state)
{
	//bool isequal = false;
	if( equal(rings.begin(), rings.end(), new_state.begin()) )
	{
		// do not change rings
		switch(status.size())
		{
		case 10:
			status = "Stationary";
			break;
		case 7:
			status = "onering";
			break;
		case 8:
			status = "tworings";
			break;
		case 11:
			status = "three_rings";
			break;
		case 9:
			status = "fourrings";
			break;
		case 6:
			status = "5rings";
			break;
		case 12:
			status = "Allringsmoved";
			break;
		case 5:
			status = "1back";
			break;
		case 4:
			status = "2bac";
			break;
		case 3:
			status = "3ba";
			break;
		case 2:
			status = "4b";
			break;
		case 1:
			status = "5";
			break;
		}
	}
	else
	{
		this-> rings = new_state;
		status = "Updated";
		cout << "I am in the loop of update" << endl;
	}
}



