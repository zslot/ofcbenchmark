/*
 * topology.hpp
 *
 *  Created on: Apr 13, 2012
 *      Author: Zsolt Magyari
 */

#ifndef TOPOLOGY_HPP_
#define TOPOLOGY_HPP_



#define MAX_SWITCH_COUNT  400

#include <iostream>
#include <fstream>
#include <vector>
#include "minIni.h"
#include "switch.hpp"

struct wired{
	int connected;
};



class topology{
public:
	void **all_switches;

	void clear();
	//void GetConnections(int _switch, vector <int>& retVal);
	void GetConnection(int _switch, int port, int& remote_switch, int& remote_port);
	void SetConnections(string left_switch,string left_port, string right_switch, string right_port);
	void detach_by_char(string& superstring, char chr, string& left, string& right);
	void LoadFromFile(char* filename);
	void Print(int count);
	void PrintConnection(int _switch, int port);

private:
	//topology matrix
	struct wired switches[MAX_SWITCH_COUNT][MAX_SWITCH_COUNT];


};

void topology::clear(){
	for (int i=0; i< MAX_SWITCH_COUNT;i++){
		for (int j=0; j<MAX_SWITCH_COUNT; j++){
			switches[i][j].connected = 0;
		}
	}
}

//void topology::GetConnections(int _switch, vector <int>& retVal){
//	vector <int> v;
//	for (int i=0; i<MAX_SWITCH_COUNT; i++){
//		if (switches[i][_switch].connected){
//		    v.push_back(i);
//		}
//	}
//	retVal.swap(v);
//}

/**
 * Returns the connected switch number to the specified port, returns remote_switch, remote_port
 */
void topology::GetConnection(int _switch, int port, int& remote_switch, int& remote_port)
{
	int i;
	bool found = false;
	for (i=0; i<MAX_SWITCH_COUNT; i++){
		if (switches[_switch][i].connected == port){
			found = true;
			break;
		}
	}
	if (found){
		remote_switch = i;
		remote_port = switches[i][_switch].connected;
	}else{
		remote_switch = 0;
		remote_port   = 0;
	}
}

void topology::SetConnections(string left_switch,string left_port, string right_switch, string right_port)
{
	char *rest;
	long ls = strtol(left_switch.c_str(), &rest, 10);
	long lp = strtol(left_port.c_str(), &rest, 10);
	long rs = strtol(right_switch.c_str(), &rest, 10);
	long rp = strtol(right_port.c_str(), &rest, 10);
	printf("%d:%d=%d:%d\n", ls, lp, rs, rp);
	switches[ls][rs].connected = lp;
	switches[rs][ls].connected = rp;
}

/**
 * Detach string
 */
void topology::detach_by_char(string& superstring, char chr, string& left, string& right)
{
	size_t pos = superstring.find(chr);
	if (pos > 0){
		left.insert(0,superstring.substr(0,pos));
		right.insert(0,superstring.substr(pos+1,superstring.length()-pos));
	}
}

/**
 * Topology from file
 */
void topology::LoadFromFile(char* filename){
	//printf("in\n");
	clear();
	try{
		ifstream fil;
		fil.open(filename);
		string line;
		while (fil.good())
		{
			getline(fil, line);
			//cout << line << endl;
			size_t pos = line.find('=');
			char *rest;
			if (pos>0){
				string str_left; //= line.substr(0,pos);
				string str_right; //= line.substr(pos+1,line.length()-pos);
				detach_by_char(line, '=', str_left,str_right);
				//cout << "left: " << str_left << endl;
				//cout << "right: " << str_right << endl;
				string left_switch;
				string left_port;
				string right_switch;
				string right_port;
				detach_by_char(str_left, ':', left_switch,left_port);
				detach_by_char(str_right, ':', right_switch,right_port);
				//cout << "left_switch: " << left_switch << endl;
				//cout << "left_port: " << left_port << endl;
				//cout << "right_switch: " << right_switch << endl;
				//cout << "right_port: " << right_port << endl;

				//long left = strtol(str_left.c_str(), &rest, 10);
				//long right =  strtol(str_right.c_str(), &rest, 10);
				//cout << left << endl;
				//cout << right << endl;
				SetConnections(left_switch,left_port, right_switch, right_port);
			}
		}
	}catch(exception e){
		printf("Exception: %s", e.what());
	}

}

/**
 * Print to screen
 */
void topology::Print(int count){
	//if (!count){
	//	count = MAX_SWITCH_COUNT;
	//}
	cout << "[Topology matrix]" << endl;
	int max = 0;
	for (int i=1; i< MAX_SWITCH_COUNT;i++){
		for (int j=1; j<MAX_SWITCH_COUNT; j++){
			if ( (switches[i][j].connected > 0) && (i>max || j>max) ){
				max = (i>j ? i : j);
			}
		}
	}
	cout << "  ";
	for (int i=1; i<= max;i++) cout << i;
	cout << endl;
	cout << "  ";
	for (int i=1; i<= max;i++) cout << "_";
		cout << endl;

	for (int i=1; i<= max;i++){
		cout << i << "|";
		for (int j=1; j<=max; j++){
			cout << switches[i][j].connected;
		}
		cout << endl;
	}
}

/**
 * Print to screen
 */
void topology::PrintConnection(int _switch, int port)
{
	int remote_switch;
	int remote_port;
	GetConnection(_switch, port, remote_switch, remote_port);
	cout << "sw: "<< _switch<<"pt:" << port<< "r_sw: " << remote_switch<< "r_pt: "<< remote_port<<endl;


	//vector <int> v;
	//GetConnections(id, v);
	//for (unsigned int i=0; i<v.size();i++)
	//{
	//	printf("Link to %d: %d\n", id,v[i]);
	//}
}


topology topo;



#endif /* TOPOLOGY_HPP_ */
