/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <functional>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"
#include "myHashMap.h"

extern CirMgr *cirMgr;

class CirMgr
{
  friend class CirGate;
public:
   
   CirMgr() : Gate_List(0), PI_List(0), AIG_List(0), PO_List(0), FECmap(0){}
   ~CirMgr() {
    if(Gate_List != NULL) delete[] Gate_List;
    if(FECmap != NULL) delete FECmap;
    if(PI_List != NULL) delete[] PI_List;
    if(PO_List != NULL) delete[] PO_List;
    if(AIG_List != NULL) delete[] AIG_List;
    if(pattern.size()!=0)pattern.clear();
   }
   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const {
    if( Gate_List[gid].getTypeStr() == "UNDEF" && Gate_List[gid].fanout_List.size() == 0)return 0;
    else return &(Gate_List[gid]);
  }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();
   void mergeAIG(size_t, size_t);
   void deletefanout(size_t, size_t);

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();
   void fsim();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() ;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() ;
   void writeAag(ostream&) ;
   void writeGate(ostream&, CirGate*);
   void traversal(CirGate&);
   void DFSAList(CirGate&);
   void updateDFS();
   void cirwg(CirGate&, vector<size_t>&, vector<size_t>&, vector<size_t>&);

   class GateNode
   {
     public:
      GateNode();
      GateNode(size_t id) : ID(id), in_1(cirMgr->getGate(id)->fanin_1), in_2(cirMgr->getGate(id)->fanin_2) {}
      ~GateNode() {}
 
      bool operator == (const GateNode& n) const { return  (( (in_1 == n.in_1) && (in_2 == n.in_2) ) || ( (in_2 == n.in_1) && (in_1 == n.in_2) ));}
      bool simoperator (const GateNode& n) const { return (ID == n.ID);}
      bool operator > (const GateNode& n) const {return (ID > n.ID);}
      bool operator < (const GateNode& n) const {return (ID < n.ID);}
      size_t operator () () const { return (in_1 + in_2); }

      size_t GateID() const { return ID; }
 
   private:
      size_t   ID;
      size_t   in_1;
      size_t   in_2;
    };
   class strashkey
  {
  public:
      strashkey(size_t id): in_1(cirMgr->getGate(id)->fanin_1), in_2(cirMgr->getGate(id)->fanin_2){}
   
      size_t operator() () const { return (in_1 * in_2); }

      bool operator == (const strashkey& n) const { return  (( (in_1 == n.in_1) && (in_2 == n.in_2) ) || ( (in_2 == n.in_1) && (in_1 == n.in_2) )); }

  private:
    size_t  in_1;
    size_t  in_2;
  };
  class FECkey
  {
  public:
      FECkey(size_t id) : ID(id){hash<unsigned long long> long_hash; key = long_hash(cirMgr->getGate(id)->simulate);}
   
      unsigned long operator() () const { return key; }

      bool operator == (const FECkey& n) const { return  ((key == n.key) && (ID == n.ID));}

  private:
    unsigned long key;
    size_t        ID;
  };

private:
  ofstream           *_simLog;
  vector<size_t>   DFS_List;
  CirGate*   Gate_List;
  HashMap<FECkey,GateNode>*     FECmap;
  map<size_t, vector<size_t>>   FECgrp;
  unsigned*   PI_List;
  unsigned*   AIG_List;
  unsigned*   PO_List;
  int     M;
  int     I;
  int     L;
  int     O;
  int     A;
  vector<string>   pattern;

};

#endif // CIR_MGR_H
