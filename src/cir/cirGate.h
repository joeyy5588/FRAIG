/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
  friend class CirMgr;
public:
    
   CirGate(): type("UNDEF"), Line_No(0), symbol(""), fanin_1(-1), fanin_2(-1), printed(false), simulate(0), IFEC(false), FEC_ID(-1), DFS_NO(-1), fraigsim(false) {}
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const { return type; }
   unsigned getLineNo() const { return Line_No; }

   // Printing functions
   //virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) ;
   void reportFanout(int level) ;
   void traversalreport(CirGate&, int, int) ;
   void traversalreport2(CirGate&, int, int) ;
   bool isAig() const {return(type == "AIG");}

private:
  string                   type;
  unsigned                 Line_No;
  string                   symbol;
  int                      fanin_1;
  int                      fanin_2;
  vector<unsigned>         fanout_List;
  mutable bool             printed;
  unsigned long long       simulate;
  bool                     IFEC;
  size_t                   FEC_ID;
  Var                      satvar;
  size_t                   DFS_NO;
  bool                     fraigsim;

protected:

};

#endif // CIR_GATE_H
