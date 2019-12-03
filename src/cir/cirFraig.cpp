/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <cstdlib>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed

void
CirMgr::strash()
{
	HashMap<strashkey,GateNode> _hashmap(getHashSize(DFS_List.size()));
	for(size_t i = 0; i < DFS_List.size(); i++){
		if(Gate_List[DFS_List[i]].type == "AIG"){
			GateNode temp(DFS_List[i]);
			strashkey tmp(DFS_List[i]);
			if(!_hashmap.strinsert(tmp, temp)){
				_hashmap.query(tmp, temp);
				mergeAIG(2*temp.GateID(), DFS_List[i]);
				A--;
				cout << "Strashing: " << temp.GateID() << " merging " << DFS_List[i] << "..." << endl;
			}
		}
	}
	updateDFS();
}

void
CirMgr::fraig()
{
	SatSolver solver;
   	solver.initialize();
   	Gate_List[0].satvar = solver.newVar();
   	solver.assumeProperty(Gate_List[0].satvar, false);
   	for(int i = 1 ; i < M+1+O; i++){
   		if(Gate_List[i].type == "UNDEF")Gate_List[i].satvar = Gate_List[0].satvar;
   		else if(Gate_List[i].type == "PO")Gate_List[i].satvar = Gate_List[Gate_List[i].fanin_1 >> 1].satvar;
   		else Gate_List[i].satvar = solver.newVar();
   	}
   	for(int i = 1 ; i < M+1; i++){
   		if(Gate_List[i].type == "AIG"){
   			solver.addAigCNF(Gate_List[i].satvar, Gate_List[Gate_List[i].fanin_1 >> 1].satvar, Gate_List[i].fanin_1 & 1, Gate_List[Gate_List[i].fanin_2 >> 1].satvar, Gate_List[i].fanin_2 & 1);
   		}
   	}
   	pattern.clear();
   	for(int i = 0 ; i < I ; i++){
   		pattern.push_back("");
   	}
   	for(size_t i = 0 ; i < DFS_List.size(); i++){
   		if((Gate_List[DFS_List[i]].type == "AIG" || Gate_List[DFS_List[i]].type == "CONST")&&(Gate_List[DFS_List[i]].FEC_ID!=-1)&&(Gate_List[DFS_List[i]].FEC_ID!=DFS_List[i])){
   				Var temp = solver.newVar();
	        	bool result;
	        	solver.addXorCNF(temp, Gate_List[Gate_List[DFS_List[i]].FEC_ID].satvar, Gate_List[Gate_List[DFS_List[i]].FEC_ID].IFEC ^ Gate_List[DFS_List[i]].IFEC , Gate_List[DFS_List[i]].satvar, false );
				solver.assumeRelease();
				solver.assumeProperty(Gate_List[0].satvar, false);
	        	solver.assumeProperty(temp, true);
	        	result = solver.assumpSolve();
	        	if(!result){
	        		//cerr << "UNSAT: " << DFS_List[i] << " " << Gate_List[DFS_List[i]].FEC_ID << endl;
	        		size_t replace = (Gate_List[DFS_List[i]].FEC_ID << 1);
	        		size_t origin = DFS_List[i];
	        		replace += (Gate_List[Gate_List[DFS_List[i]].FEC_ID].IFEC ^ Gate_List[DFS_List[i]].IFEC ? 1 : 0);
	        		deletefanout(Gate_List[origin].fanin_1/2, origin);
	        		deletefanout(Gate_List[origin].fanin_2/2, origin);
	        		mergeAIG(replace, origin);
	        		A--;
	        		cout << "Fraig: " << Gate_List[DFS_List[i]].FEC_ID << " " << "merging " ;
	        		if(replace&1)cout << "!";
	        		cout << origin << "..." << endl;
	        		for(size_t e = 0 ; e < FECgrp[Gate_List[DFS_List[i]].FEC_ID].size(); e++){
	        			if(FECgrp[Gate_List[DFS_List[i]].FEC_ID][e] == origin){
	        				FECgrp[Gate_List[DFS_List[i]].FEC_ID].erase(FECgrp[Gate_List[DFS_List[i]].FEC_ID].begin() + e);
	        			}
	        		}
	        		if(FECgrp[Gate_List[DFS_List[i]].FEC_ID].size() < 2)FECgrp.erase(Gate_List[DFS_List[i]].FEC_ID);
	        	}else{
	        		//cerr << "SAT: " << DFS_List[i] << " " << Gate_List[DFS_List[i]].FEC_ID << endl;
	        		/*for(size_t e = 0 ; e < FECgrp[Gate_List[DFS_List[i]].FEC_ID].size(); e++){
	        			if(FECgrp[Gate_List[DFS_List[i]].FEC_ID][e] == DFS_List[i]){
	        				FECgrp[Gate_List[DFS_List[i]].FEC_ID].erase(FECgrp[Gate_List[DFS_List[i]].FEC_ID].begin() + e);
	        			}
	        		}
	        		if(FECgrp[Gate_List[DFS_List[i]].FEC_ID].size() < 2)FECgrp.erase(Gate_List[DFS_List[i]].FEC_ID);*/
	        		for(int t = 0 ; t < I; t++){
	        			//cerr << t << " " << solver.getValue(Gate_List[*(PI_List+t)].satvar) << endl;
	        			if(solver.getValue(Gate_List[*(PI_List+t)].satvar) == 1){Gate_List[(*(PI_List+t))].simulate = 1; }
	        			else if(solver.getValue(Gate_List[*(PI_List+t)].satvar) == 0){Gate_List[(*(PI_List+t))].simulate = 0;}
	        		}
	        		fsim();
	        	}

   		   	}
    }

	updateDFS();
	
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::fsim()
{
	for(size_t k = 0; k < DFS_List.size(); k++){
		//if(Gate_List[DFS_List[k]].type == "PI") cerr << DFS_List[k] << " " << Gate_List[DFS_List[k]].simulate << endl;
		if(Gate_List[DFS_List[k]].type == "AIG"){
			if(Gate_List[DFS_List[k]].fanin_1&1){
				if(Gate_List[DFS_List[k]].fanin_2&1){
					Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate ^ 1)&&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate ^ 1));
				}
				else{
					Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate ^ 1)&&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
				}
			}else if(Gate_List[DFS_List[k]].fanin_2&1){
				Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate ^ 1));
			}else{
				Gate_List[DFS_List[k]].simulate = ((Gate_List[Gate_List[DFS_List[k]].fanin_1/2].simulate)&&(Gate_List[Gate_List[DFS_List[k]].fanin_2/2].simulate));
			}
			Gate_List[DFS_List[k]].fraigsim = true;
		}
	}
	for(map<size_t, vector<size_t>>::iterator it = FECgrp.begin(); it != FECgrp.end(); it++){
		
		for(size_t i = 1 ; i < it -> second.size(); i++){
			if(((Gate_List[it -> second[0]].simulate != Gate_List[it -> second[i]].simulate)&&((Gate_List[it -> second[0]].IFEC == Gate_List[it -> second[i]].IFEC))) ||
				((Gate_List[it -> second[0]].simulate == Gate_List[it -> second[i]].simulate)&&((Gate_List[it -> second[0]].IFEC != Gate_List[it -> second[i]].IFEC)))){
				//cerr << "collsion" << it -> second[0] << " " << it -> second[i] << endl;
				bool insert = false;
				for(size_t j = i+1 ; j < it -> second.size(); j++){
					if(((Gate_List[it -> second[i]].simulate == Gate_List[it -> second[j]].simulate)&&(Gate_List[it -> second[i]].IFEC == Gate_List[it -> second[j]].IFEC)) ||
						((Gate_List[it -> second[i]].simulate != Gate_List[it -> second[j]].simulate)&&(Gate_List[it -> second[i]].IFEC != Gate_List[it -> second[j]].IFEC))){
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
	//printFECPairs();
}