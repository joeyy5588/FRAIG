/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <typeinfo>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
	vector<size_t> temp = DFS_List;
	sort(temp.begin(), temp.end());
	size_t i = 0, j = 0;
	while(i < temp.size()){
		//cerr << "out" <<  temp[i] << " " << j << endl;
		if(temp[i] == j){
			//cerr << i << " " << j << endl;
			i++;
			j++;
		}else{
			if(Gate_List[j].type == "AIG"){
				A--;
				for(size_t r1 = 0 ; r1 < Gate_List[Gate_List[j].fanin_1/2].fanout_List.size(); r1++){
					if(Gate_List[Gate_List[j].fanin_1/2].fanout_List[r1]/2 == j){
						Gate_List[Gate_List[j].fanin_1/2].fanout_List.erase(Gate_List[Gate_List[j].fanin_1/2].fanout_List.begin()+r1);
						break;
					}
				}
				for(size_t r1 = 0 ; r1 < Gate_List[Gate_List[j].fanin_2/2].fanout_List.size(); r1++){
					if(Gate_List[Gate_List[j].fanin_2/2].fanout_List[r1]/2 == j){
						Gate_List[Gate_List[j].fanin_2/2].fanout_List.erase(Gate_List[Gate_List[j].fanin_2/2].fanout_List.begin()+r1);
						break;
					}
				}
				Gate_List[j].type = "UNDEF";
				cout << "Sweeping: AIG(" << j << ") removed..." << endl;
				if(Gate_List[Gate_List[j].fanin_1/2].type == "UNDEF" && 
				Gate_List[Gate_List[j].fanin_1/2].fanin_1 ==-1 && Gate_List[Gate_List[j].fanin_1/2].fanin_2 == -1)cout << "Sweeping: UNDEF(" << Gate_List[j].fanin_1/2 << ") removed..." << endl;
				if(Gate_List[Gate_List[j].fanin_2/2].type == "UNDEF" &&
				Gate_List[Gate_List[j].fanin_2/2].fanin_1 ==-1 && Gate_List[Gate_List[j].fanin_2/2].fanin_2 == -1)cout << "Sweeping: UNDEF(" << Gate_List[j].fanin_2/2 << ") removed..." << endl;	
			}/*else if(Gate_List[j].type == "UNDEF" && Gate_List[j].fanout_List.size() != 0){
				cout << "Sweeping: UNDEF(" << j << ") removed..." << endl;
			}*/
			j++;
		}
	}
	
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
	for(size_t i = 0; i < DFS_List.size(); i++){
		if(Gate_List[DFS_List[i]].type == "AIG"){
			if((Gate_List[DFS_List[i]].fanin_1 == 0) || (Gate_List[DFS_List[i]].fanin_2 == 0) || ((Gate_List[DFS_List[i]].fanin_1 != Gate_List[DFS_List[i]].fanin_2) && (Gate_List[DFS_List[i]].fanin_1/2 == Gate_List[DFS_List[i]].fanin_2/2))){
				//merge to 0
				cout << "Simplifying: " << 0 << " merging ";
				cout << DFS_List[i] << "..." << endl;
				deletefanout(Gate_List[DFS_List[i]].fanin_1/2, DFS_List[i]);
				deletefanout(Gate_List[DFS_List[i]].fanin_2/2, DFS_List[i]);
				mergeAIG(0, DFS_List[i]);
				A--;
			}else if((Gate_List[DFS_List[i]].fanin_1 == Gate_List[DFS_List[i]].fanin_2) || (Gate_List[DFS_List[i]].fanin_2 == 1)){
				//merge to fanin_1
				cout << "Simplifying: " << Gate_List[DFS_List[i]].fanin_1/2 << " merging ";
				if(Gate_List[DFS_List[i]].fanin_1%2)cout << "!";
				deletefanout(Gate_List[DFS_List[i]].fanin_1/2, DFS_List[i]);
				deletefanout(Gate_List[DFS_List[i]].fanin_2/2, DFS_List[i]);				
				mergeAIG((Gate_List[DFS_List[i]].fanin_1), DFS_List[i]);
				
				cout << DFS_List[i] << "..." << endl;				
				A--;
			}else if(Gate_List[DFS_List[i]].fanin_1 == 1){
				//merge to fanin_2
				cout << "Simplifying: " << Gate_List[DFS_List[i]].fanin_2/2 << " merging ";
				if(Gate_List[DFS_List[i]].fanin_2%2)cout << "!";

				
				deletefanout(Gate_List[DFS_List[i]].fanin_2/2, DFS_List[i]);
				deletefanout(Gate_List[DFS_List[i]].fanin_2/2, DFS_List[i]);
				mergeAIG((Gate_List[DFS_List[i]].fanin_2), DFS_List[i]);
				cout << DFS_List[i] << "..." << endl;
				A--;
			}
		}
	}
	updateDFS();
}

void
CirMgr::mergeAIG(size_t replace, size_t origin)//replace -> literal origin -> id
{
	size_t odd = replace%2;
	for(size_t i = 0; i < Gate_List[origin].fanout_List.size(); i++){
		if(Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_1/2 == origin && odd == 1){
			Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_1 = replace - Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_1%2;
			if(Gate_List[origin].fanout_List[i]%2 == 1)Gate_List[replace/2].fanout_List.push_back(Gate_List[origin].fanout_List[i]-1);
			else Gate_List[replace/2].fanout_List.push_back(Gate_List[origin].fanout_List[i] + 1);
		}else if(Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_1/2 == origin && odd == 0){
			Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_1 = replace + Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_1%2;
			
			Gate_List[replace/2].fanout_List.push_back(Gate_List[origin].fanout_List[i]);
		}else if(Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_2/2 == origin && odd == 1){
			Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_2 = replace - Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_2%2;
			
			if(Gate_List[origin].fanout_List[i]%2 == 1)Gate_List[replace/2].fanout_List.push_back(Gate_List[origin].fanout_List[i]-1);
			else Gate_List[replace/2].fanout_List.push_back(Gate_List[origin].fanout_List[i] + 1);
		}else if(Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_2/2 == origin && odd == 0){
			Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_2 = replace + Gate_List[Gate_List[origin].fanout_List[i]/2].fanin_2%2;
			
			Gate_List[replace/2].fanout_List.push_back(Gate_List[origin].fanout_List[i]);
		}
	}
	
	Gate_List[origin].fanout_List.clear();
	Gate_List[origin].fanin_1 = -1;
	Gate_List[origin].fanin_2 = -1;
	Gate_List[origin].type = "UNDEF";
	
}
void
CirMgr::deletefanout(size_t list, size_t element)
{
	for(size_t i = 0; i < Gate_List[list].fanout_List.size(); i++){
		if(Gate_List[list].fanout_List[i]/2 == element){
			Gate_List[list].fanout_List.erase(Gate_List[list].fanout_List.begin()+i);
			return;
		}
	}
}
/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
