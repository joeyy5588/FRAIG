/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <cstdlib>

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
	for(int i = 0; i < M+1+O; i++){
		Gate_List[i].simulate = 0;
		Gate_List[i].FEC_ID = -1;
	}
	FECgrp.clear();
	FECmap -> clear();
	GateNode temp(0);
	FECkey tmp(0);
	FECmap->insert(tmp, temp);
	size_t times = I+1;
	size_t sim_time = 0;
	while(times > 0){
		sim_time++;
		bool change = false;
		for(int j = 0; j < I; j++){
			Gate_List[*(PI_List+j)].simulate = (rand() + (rand() << 16) + (((unsigned long long)rand()) << 32) + (((unsigned long long)rand()) << 48) );
		}
		for(size_t k = 0; k < DFS_List.size(); k++){
			if((Gate_List[DFS_List[k]].type == "AIG")||(Gate_List[DFS_List[k]].type == "PO")){
				/*cerr << DFS_List[k] << " 1 " << (Gate_List[DFS_List[k]].fanin_1&1) << endl;
				cerr << DFS_List[k] << " 2 " << Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate[i] << endl;
				cerr << DFS_List[k] << " 3 " << (Gate_List[DFS_List[k]].fanin_2&1) << endl;
				cerr << DFS_List[k] << " 4 " << Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate[i] << endl;*/
				if(Gate_List[DFS_List[k]].fanin_1&1){
					if(Gate_List[DFS_List[k]].fanin_2&1){
						Gate_List[DFS_List[k]].simulate = ((~Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(~Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
					}
					else{
						Gate_List[DFS_List[k]].simulate = ((~Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
					}
				}else if(Gate_List[DFS_List[k]].fanin_2&1){
					Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(~Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
				}else{
					Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
				}
				Gate_List[DFS_List[k]].IFEC = (Gate_List[DFS_List[k]].simulate >> 63);
			}/*else if(Gate_List[DFS_List[k]].type == "PO"){
				bool in1 = (Gate_List[DFS_List[k]].fanin_1&1) ^ (Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate[i] == "1" ? 1 : 0);
				Gate_List[DFS_List[k]].simulate += ( in1 ? "1" : "0" );
			}*/
		}
		if(sim_time == 1){
			for(size_t i = 0; i < DFS_List.size(); i++){
				if(Gate_List[DFS_List[i]].type == "AIG"){
					if(Gate_List[DFS_List[i]].IFEC == 1)Gate_List[DFS_List[i]].simulate = (~Gate_List[DFS_List[i]].simulate);
					GateNode temp(DFS_List[i]);
					FECkey tmp(DFS_List[i]);
					if(!FECmap->insert(tmp, temp));//cout<<"error1 " << DFS_List[i];
				}
			}
			for(size_t i = 0; i < FECmap->numBuckets(); i++){
			    if((!(*(FECmap->_buckets+i)).empty()) && ((*(FECmap->_buckets+i)).size() > 1)){
			      for(size_t j = 0; j < (*(FECmap->_buckets+i)).size(); j++){
			          Gate_List[(*(FECmap->_buckets+i))[j].second.GateID()].FEC_ID = (*(FECmap->_buckets+i))[0].second.GateID();
			          FECgrp[(*(FECmap->_buckets+i))[0].second.GateID()].push_back((*(FECmap->_buckets+i))[j].second.GateID());
			        } 
			    }
			}
		}else{
			for(size_t i = 0; i < DFS_List.size(); i++){
				if(Gate_List[DFS_List[i]].type == "AIG"){
					if(Gate_List[DFS_List[i]].IFEC == 1)Gate_List[DFS_List[i]].simulate = (~Gate_List[DFS_List[i]].simulate);
				}
			}
			/*for(size_t i = 0; i < FECmap->numBuckets(); i++){
			    if((!(*(FECmap->_buckets+i)).empty()) && ((*(FECmap->_buckets+i)).size() > 1)){
			      size_t s = (*(FECmap->_buckets+i)).size();
			      for(size_t j = 0; j < s; j++){
			        if(j!=0){
			        	if(Gate_List[(*(FECmap->_buckets+i))[j].second.GateID()].simulate != Gate_List[(*(FECmap->_buckets+i))[0].second.GateID()].simulate){
			        		/*for(size_t v = 0; v < s; v++){
			        			cout << (*(FECmap->_buckets+i))[v].second.GateID() << " ";
			      			}
			      			//cout << endl;
			        		GateNode temp((*(FECmap->_buckets+i))[j].second.GateID());
							FECkey tmp((*(FECmap->_buckets+i))[j].second.GateID());
							//FECkey lead((*(FECmap->_buckets+i))[0].second.GateID());
							if(FECmap->insert(tmp, temp)){
								(*(FECmap->_buckets+i)).erase((*(FECmap->_buckets+i)).begin()+j);
				        		j--;
				        		s--;
							}
				        	
			        		change = true;
			        	}
			        } 
			      }
			    }
			}*/

			for(map<size_t, vector<size_t>>::iterator it = FECgrp.begin(); it != FECgrp.end(); it++){
				
				for(size_t i = 1 ; i < it -> second.size(); i++){
					if(Gate_List[it -> second[0]].simulate != Gate_List[it -> second[i]].simulate){
						//cerr << "collsion" << it -> second[0] << " " << it -> second[i] << endl;
						change = true;
						bool insert = false;
						for(size_t j = i+1 ; j < it -> second.size(); j++){
							if(Gate_List[it -> second[i]].simulate == Gate_List[it -> second[j]].simulate){
								if(insert == false){
									FECgrp[it -> second[i]].push_back(it -> second[i]);
									Gate_List[it -> second[i]].FEC_ID = it -> second[i];
								}
								FECgrp[it -> second[i]].push_back(it -> second[j]);
								insert = true;
								Gate_List[it -> second[j]].FEC_ID = it -> second[i];
								FECgrp[it -> second[0]].erase(FECgrp[it -> second[0]].begin()+j);
								j--;

							}
						}
						FECgrp[it -> second[0]].erase(FECgrp[it -> second[0]].begin()+i);
						i--;
					}
				}
			}
		}
		if(!change)times -= 1;
		//cerr << times;
		
	}
	/*FECmap -> mapsort();
	for(size_t i = 0; i < FECmap->numBuckets(); i++){
	    if((!(*(FECmap->_buckets+i)).empty()) && ((*(FECmap->_buckets+i)).size() > 1)){
	      for(size_t j = 0; j < (*(FECmap->_buckets+i)).size(); j++){
	          Gate_List[(*(FECmap->_buckets+i))[j].second.GateID()].FEC_ID = (*(FECmap->_buckets+i))[0].second.GateID();
	          FECgrp[(*(FECmap->_buckets+i))[0].second.GateID()].push_back((*(FECmap->_buckets+i))[j].second.GateID());
	        } 
	    }
	}*/
	cout << 64*sim_time << " patterns simulated." << endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	for(int i = 0; i < M+1+O; i++){
		Gate_List[i].simulate = 0;
		Gate_List[i].FEC_ID = -1;
	}
	FECgrp.clear();
	pattern.clear();
	FECmap -> clear();
	GateNode temp(0);
	FECkey tmp(0);
	FECmap->insert(tmp, temp);
	string buffer;
	size_t sim_time = 0;
	bool   first = true;
	while(getline(patternFile, buffer)){
		if(buffer.length() != I){
			cerr << "Error: Pattern(" << buffer <<") length(" << buffer.length() << ") does not match the number of inputs(" << I << ") in a circuit!!" << endl;
			pattern.clear();
			return;
		}
		if(first){
			first = false;
			for(size_t i = 0; i < I; i++){
				pattern.push_back("");
			}
		}
		for(size_t i = 0; i < I; i++){
			if((buffer[i]!= '0') && (buffer[i]!= '1')){
				cerr << "Error: Pattern(" << buffer << ") contains a non-0/1 character(‘" << buffer[i]<< "’)" << endl;
				pattern.clear();
				return;
			}
			pattern[i] += buffer[i];
		}
	}
	sim_time = pattern[0].length();
	patternFile.close();
	for(size_t sim = 0; sim < ((sim_time >> 6) + (sim_time & 63 ? 1: 0)); sim++){
		for(size_t j = 0; j < I; j++){
			Gate_List[*(PI_List+j)].simulate = 0;
		}
		if(pattern[0].length() >= (64+sim*64)  ){
			for(size_t i = 0; i < 64; i++){
				for(size_t j = 0; j < I; j++){
					
					Gate_List[*(PI_List+j)].simulate ^= (((unsigned long long)(pattern[j][i+sim*64] - '0')) << (i));
				}
			}
		}else{
			for(size_t i = 0; i < (pattern[0].length()-sim*64); i++){
				//cerr << sim ;
				for(size_t j = 0; j < I; j++){
					Gate_List[*(PI_List+j)].simulate = (Gate_List[*(PI_List+j)].simulate ^ (((unsigned long long)(pattern[j][i+sim*64] - '0')) << (i)));
				}
			}
		}
		for(size_t k = 0; k < DFS_List.size(); k++){
			if((Gate_List[DFS_List[k]].type == "AIG")||(Gate_List[DFS_List[k]].type == "PO")){
				/*cerr << DFS_List[k] << " 1 " << (Gate_List[DFS_List[k]].fanin_1&1) << endl;
				cerr << DFS_List[k] << " 2 " << Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate[i] << endl;
				cerr << DFS_List[k] << " 3 " << (Gate_List[DFS_List[k]].fanin_2&1) << endl;
				cerr << DFS_List[k] << " 4 " << Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate[i] << endl;*/
				if(Gate_List[DFS_List[k]].fanin_1&1){
					if(Gate_List[DFS_List[k]].fanin_2&1){
						Gate_List[DFS_List[k]].simulate = ((~Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(~Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
					}
					else{
						Gate_List[DFS_List[k]].simulate = ((~Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
					}
				}else if(Gate_List[DFS_List[k]].fanin_2&1){
					Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(~Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
				}else{
					Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
				}
				Gate_List[DFS_List[k]].IFEC = (Gate_List[DFS_List[k]].simulate >> 63);
			}/*else if(Gate_List[DFS_List[k]].type == "PO"){
				bool in1 = (Gate_List[DFS_List[k]].fanin_1&1) ^ (Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate[i] == "1" ? 1 : 0);
				Gate_List[DFS_List[k]].simulate += ( in1 ? "1" : "0" );
			}*/
		}
		if(sim == 0){
			for(size_t i = 0; i < DFS_List.size(); i++){
				if(Gate_List[DFS_List[i]].type == "AIG"){
					if(Gate_List[DFS_List[i]].IFEC){Gate_List[DFS_List[i]].simulate = (~Gate_List[DFS_List[i]].simulate);}
					GateNode temp(DFS_List[i]);
					FECkey tmp(DFS_List[i]);
					if(!FECmap->insert(tmp, temp));//cout<<"error1 " << DFS_List[i];
				}
			}
			for(size_t i = 0; i < FECmap->numBuckets(); i++){
			    if((!(*(FECmap->_buckets+i)).empty()) && ((*(FECmap->_buckets+i)).size() > 1)){
			      for(size_t j = 0; j < (*(FECmap->_buckets+i)).size(); j++){
			          Gate_List[(*(FECmap->_buckets+i))[j].second.GateID()].FEC_ID = (*(FECmap->_buckets+i))[0].second.GateID();
			          FECgrp[(*(FECmap->_buckets+i))[0].second.GateID()].push_back((*(FECmap->_buckets+i))[j].second.GateID());
			        } 
			    }
			}
		}else{
			for(size_t i = 0; i < DFS_List.size(); i++){
				if(Gate_List[DFS_List[i]].type == "AIG"){
					if(Gate_List[DFS_List[i]].IFEC == 1)Gate_List[DFS_List[i]].simulate = (~Gate_List[DFS_List[i]].simulate);
				}
			}
			/*for(size_t i = 0; i < FECmap->numBuckets(); i++){
			    if((!(*(FECmap->_buckets+i)).empty()) && ((*(FECmap->_buckets+i)).size() > 1)){
			      size_t s = (*(FECmap->_buckets+i)).size();
			      for(size_t j = 0; j < s; j++){
			        if(j!=0){
			        	if(Gate_List[(*(FECmap->_buckets+i))[j].second.GateID()].simulate != Gate_List[(*(FECmap->_buckets+i))[0].second.GateID()].simulate){
			        		//cerr << (*(FECmap->_buckets+i))[j].second.GateID() << " " << (*(FECmap->_buckets+i))[0].second.GateID() << endl;
			        		GateNode temp((*(FECmap->_buckets+i))[j].second.GateID());
							FECkey tmp((*(FECmap->_buckets+i))[j].second.GateID());
							if(FECmap->insert(tmp, temp)){
								(*(FECmap->_buckets+i)).erase((*(FECmap->_buckets+i)).begin()+j);
				        		j--;
				        		s--;
							}
			        	}
			        } 
			      }
			    }
			}*/
			for(map<size_t, vector<size_t>>::iterator it = FECgrp.begin(); it != FECgrp.end(); it++){
				
				for(size_t i = 1 ; i < it -> second.size(); i++){
					if(Gate_List[it -> second[0]].simulate != Gate_List[it -> second[i]].simulate){
						//cerr << "collsion" << it -> second[0] << " " << it -> second[i] << endl;
						bool insert = false;
						for(size_t j = i+1 ; j < it -> second.size(); j++){
							if(Gate_List[it -> second[i]].simulate == Gate_List[it -> second[j]].simulate){
								if(insert == false){
									FECgrp[it -> second[i]].push_back(it -> second[i]);
									Gate_List[it -> second[i]].FEC_ID = it -> second[i];
								}
								FECgrp[it -> second[i]].push_back(it -> second[j]);
								insert = true;
								Gate_List[it -> second[j]].FEC_ID = it -> second[i];
								FECgrp[it -> second[0]].erase(FECgrp[it -> second[0]].begin()+j);
								j--;

							}
						}
						FECgrp[it -> second[0]].erase(FECgrp[it -> second[0]].begin()+i);
						i--;
					}
				}
			}
		}

	}

	cout << sim_time << " patterns simulated." << endl;
	/*FECmap -> mapsort();
	for(size_t i = 0; i < FECmap->numBuckets(); i++){
	    if((!(*(FECmap->_buckets+i)).empty()) && ((*(FECmap->_buckets+i)).size() > 1)){
	      for(size_t j = 0; j < (*(FECmap->_buckets+i)).size(); j++){
	          Gate_List[(*(FECmap->_buckets+i))[j].second.GateID()].FEC_ID = (*(FECmap->_buckets+i))[0].second.GateID();
	          FECgrp[(*(FECmap->_buckets+i))[0].second.GateID()].push_back((*(FECmap->_buckets+i))[j].second.GateID());
	        } 
	    }
	}*/
	if(_simLog){
		for(size_t i = 0; i < (64 > sim_time ? sim_time : 64); i++){
			for(size_t j = 0; j < I; j++){
				(*_simLog) << pattern[j][i];
			}
			(*_simLog) << " ";
			for(size_t j = 0; j < O; j++){
				(*_simLog) << ((Gate_List[M+1+j].simulate >> i)&1);
			}
			(*_simLog) << endl;
		}
	}
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
