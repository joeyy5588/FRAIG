/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
	cout << "================================================================================" << endl;
	
	//cout << right << setw(50) << " =" << '\r';
	cout << "= " << type << "(" << (this-(cirMgr->Gate_List)) << ")" ;
	if(symbol != "")cout << "\"" << symbol << "\"" ;
	cout << ", line " << Line_No << endl;
	cout << "= " << "FECs:" ;
	if(!fraigsim){
		for(size_t i = 0; i < cirMgr->FECgrp[FEC_ID].size(); i++){
		if(cirMgr-> FECgrp[FEC_ID][i] != (this-(cirMgr->Gate_List))){
			cout << " ";
			if(cirMgr -> Gate_List[cirMgr-> FECgrp[FEC_ID][i]].IFEC ^ IFEC)cout << "!";
			cout << cirMgr-> FECgrp[FEC_ID][i];
		} 
	}
	}
	cout << endl;
	cout << "= " << "Value: " ;
	for(size_t i = 0; i < 64; i++){
		if((i!=0) && (i%8 == 0))cout << "_";
		if(!fraigsim){
			if(!IFEC)cout << ((simulate >> (63-i))&1) ;
			else cout << (((simulate >> (63-i))&1)^1) ;
		}else{
			cout << ((simulate >> (63-i))&1) ;
		}
		
	}
	cout << endl;
	cout << "================================================================================" << endl;
}

void 
CirGate::traversalreport(CirGate& output, int level, int firstlevel)
{
	int odd = 0;
	bool skip = false;
	
	if(output.fanin_1 != -1 ){
		odd = output.fanin_1 % 2;
		//if(output.type == "PO")odd = output.literal%2;
		for(int i = 0; i < (firstlevel - level) * 2 + 2; i++){ cout << " ";}
		if(odd)cout << "!";
		cout << cirMgr -> Gate_List[output.fanin_1/2].type << " " << output.fanin_1/2;
		if(cirMgr -> Gate_List[output.fanin_1/2].printed && level > 1 ){
			if(cirMgr -> Gate_List[output.fanin_1/2].type == "AIG")cout << " (*)" ; 
			skip = true;
		} 
		cout << endl;
		
	}
	if(output.fanin_1 != -1 && level > 1)cirMgr -> Gate_List[output.fanin_1/2].printed = true;
	if(output.fanin_1!=-1 && cirMgr -> Gate_List[output.fanin_1/2].type != "UNDEF" && level > 1 && !skip) traversalreport(cirMgr -> Gate_List[output.fanin_1/2],level - 1, firstlevel);
	
	if(output.fanin_2 != -1 ){
		odd = output.fanin_2 % 2;
		//if(output.type == "PO")odd = output.literal%2;
		for(int i = 0; i < (firstlevel - level) * 2 + 2; i++){ cout << " ";}
		if(odd)cout << "!";
		cout << cirMgr -> Gate_List[output.fanin_2/2].type << " " << output.fanin_2/2;
		if(cirMgr -> Gate_List[output.fanin_2/2].printed && level > 1){
			if(cirMgr -> Gate_List[output.fanin_2/2].type == "AIG")cout << " (*)" ;
			cout << endl; 
			return;
		}
		cout << endl;
		
	}
	
	if(output.fanin_2 != -1 && level > 1)cirMgr -> Gate_List[output.fanin_2/2].printed = true;
	if(output.fanin_2!=-1 && cirMgr -> Gate_List[output.fanin_2/2].type != "UNDEF" && level > 1) traversalreport(cirMgr -> Gate_List[output.fanin_2/2],level - 1, firstlevel);
	
	
	//level -- ;
}

void 
CirGate::traversalreport2(CirGate& output, int level, int firstlevel)
{
	bool skip = false;
	for(size_t i = 0; i < output.fanout_List.size(); i++){
		for(int j = 0; j < (firstlevel - level) * 2 + 2; j++){ cout << " ";}
		if(cirMgr -> Gate_List[output.fanout_List[i]/2].type == "PO"){if(output.fanout_List[i] % 2) cout << "!";}
		else if(cirMgr -> Gate_List[output.fanout_List[i]/2].type == "AIG"){if(output.fanout_List[i] % 2) cout << "!";}
		cout << cirMgr -> Gate_List[output.fanout_List[i]/2].type << " " << output.fanout_List[i]/2;
		if(cirMgr -> Gate_List[output.fanout_List[i]/2].printed && level > 1){
			if(cirMgr -> Gate_List[output.fanout_List[i]/2].type == "AIG")cout << " (*)" ; 
			skip = true;
		}
		cout << endl;
		if(level > 1)cirMgr -> Gate_List[output.fanout_List[i]/2].printed = true;
		if(cirMgr -> Gate_List[output.fanout_List[i]/2].fanout_List.size()!=0 && level > 1 && !skip)traversalreport2(cirMgr -> Gate_List[output.fanout_List[i]/2], level - 1 , firstlevel);
	}
}

void
CirGate::reportFanin(int level) 
{
   assert (level >= 0);
   for(int i = 0; i< (cirMgr -> M+1+cirMgr -> O); i++){
		cirMgr -> Gate_List[i].printed = false;
	}
   cout << type << " " << (this-(cirMgr->Gate_List)) << endl;
   traversalreport(*this, level, level);
   for(int i = 0; i< (cirMgr -> M+1+cirMgr -> O); i++){
		cirMgr -> Gate_List[i].printed = false;
	}
}

void
CirGate::reportFanout(int level) 
{
   assert (level >= 0);
   for(int i = 0; i< (cirMgr -> M+1+cirMgr -> O); i++){
		cirMgr -> Gate_List[i].printed = false;
	}
   cout << type << " " << (this-(cirMgr->Gate_List)) << endl;
   traversalreport2(*this, level, level);
   for(int i = 0; i< (cirMgr -> M+1+cirMgr -> O); i++){
		cirMgr -> Gate_List[i].printed = false;
	}
}
