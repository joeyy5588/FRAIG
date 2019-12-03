/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
  lineNo = 0;
   colNo = 0;
   ifstream _cirfile;
   string token;
   string buffer;
   int spacecount = 0;
   size_t wspace_pos;
   size_t space_pos[6]={};
   space_pos[0] = -1;
   _cirfile.open(fileName.c_str(),ifstream::in);
   if(!_cirfile.is_open()){
    cerr<<"Cannot open design \""<<fileName<<"\"!!"<<endl;
    return false;
   }
   if(!getline(_cirfile, buffer)){
    errMsg = "aag";
    return parseError(MISSING_IDENTIFIER);
   }
   _cirfile.close();
   for(size_t i = 0;i < 6; i++){
    space_pos[spacecount+1] = buffer.find(' ') + space_pos[spacecount] + 1;
    spacecount++;
    //cerr<<space_pos[spacecount]<<endl;
    wspace_pos = buffer.find('\t') + space_pos[spacecount-1] + 1;
    if(wspace_pos < space_pos[spacecount] && buffer.find('\t')!=string::npos){
      errInt = '\t';
      colNo = wspace_pos;
      return parseError(ILLEGAL_WSPACE);
    }
    token = buffer.substr(0, space_pos[spacecount]-1-space_pos[spacecount-1]);
    if(i == 0){
      if(token == "")return parseError(EXTRA_SPACE);
      else{
        if(token.length() == 3){
          if(myStrNCmp(token,"aag",3)!=0){
            errMsg = token;
            return parseError(ILLEGAL_IDENTIFIER);
          }else if(buffer.find(' ') == string::npos){
            colNo = 3;
            errMsg = "number of variables";
            return parseError(MISSING_NUM);
          }else if(buffer.length() == 4){
            colNo = 4;
            errMsg = "number of variables";
            return parseError(MISSING_NUM);
          }
        }else if(token.length() > 3){
          if(token[3]>='0'&&token[3]<='9'){
            colNo= 3;
            return parseError(MISSING_SPACE);
          }else{
            errMsg = token;
            return parseError(ILLEGAL_IDENTIFIER);
          }
        }else{
          errMsg = token;
          return parseError(ILLEGAL_IDENTIFIER);
        }
      }
    }else{
      if(token == ""){
        colNo = space_pos[spacecount];
        return parseError(EXTRA_SPACE);
      }
      if(i == 1){
        if(!myStr2Int(token, M) || M < 0){
          errMsg = "number of variables(" + token + ")";
          return parseError(ILLEGAL_NUM);
        }
      }else if(i == 2){
        if(!myStr2Int(token, I) || I < 0){
          errMsg = "number of PIs(" + token + ")";
          return parseError(ILLEGAL_NUM);
        }
      }else if(i == 3){
        if(!myStr2Int(token, L) || L < 0){
          errMsg = "number of latches(" + token + ")";
          return parseError(ILLEGAL_NUM);
        }
      }else if(i == 4){
        if(!myStr2Int(token, O) || O < 0){
          errMsg = "number of POs(" + token + ")";
          return parseError(ILLEGAL_NUM);
        }
      }else if(i == 5){
        if(!myStr2Int(token, A) || A < 0){
          errMsg = "number of AIGs(" + token + ")";
          return parseError(ILLEGAL_NUM);
        }else if(buffer.length()>token.length()){
          colNo = (space_pos[spacecount-1] +1 + token.length());
          return parseError(MISSING_NEWLINE);
        }
      }
    }
    buffer = buffer.substr(space_pos[spacecount]-space_pos[spacecount-1]);
    //cerr << buffer << endl;
   }
   buffer.clear();
   if(M<I+A+L){
    errMsg = "Number of variables";
    errInt = M;
    return parseError(NUM_TOO_SMALL);
   }
   if(L > 0){
    errMsg = "number of latches(" + token + ")";
    return parseError(ILLEGAL_NUM);
   }
   lineNo++;
   colNo = 0;
   ifstream cirfile;
   cirfile.open(fileName.c_str(),ifstream::in);
   getline(cirfile,buffer);
   buffer.clear();

   Gate_List = new CirGate[M+O+1];
   FECmap = new HashMap<FECkey, GateNode>(getHashSize(M+1+O));
   PI_List = new unsigned[I];
   PO_List = new unsigned[O];
   AIG_List = new unsigned[A];
   Gate_List[0].type = "CONST";
   Gate_List[0].simulate = 0;

   //Read PI
   for(int i = 0; i < I; i++){
    int Gate_ID;
    if(!getline(cirfile,buffer)){
      errMsg = "PI";
      return parseError(MISSING_DEF);
    }
    //cerr<<i<<endl;
    if(buffer.empty()){
      errMsg = "PI literal ID";
      return parseError(MISSING_NUM);
    }
    if(buffer.find(' ')!=string::npos){
      if(buffer[0] == ' '){
        return parseError(EXTRA_SPACE);
      }else{
        colNo+= buffer.find(' ');
        return parseError(MISSING_NEWLINE);
      }
    }
    //cerr<<i+1<<endl;
    if(buffer.find('\t')!=string::npos){
      if(buffer[0] == '\t'){
        errInt = '\t';
        colNo = buffer.find('\t');
        return parseError(ILLEGAL_WSPACE);
      }else{
        colNo+= buffer.find(' ');
        return parseError(MISSING_NEWLINE);
      }
    }
    //cerr<<i+2<<endl;
    if(!myStr2Int(buffer, Gate_ID) || Gate_ID < 0){
      errMsg = "PI literal ID(" + buffer + ")";
      return parseError(ILLEGAL_NUM);
    }
    if(Gate_ID/2 == 0){
      errInt = Gate_ID;
      return parseError(REDEF_CONST);
    }
    if(Gate_ID/2 > M){
      errInt = Gate_ID;
      return parseError(MAX_LIT_ID);
    }
    if(Gate_ID%2 == 1){
      errMsg = "PI";
      return parseError(CANNOT_INVERTED);
    }
    //cerr<<i+3<<endl;
    if(Gate_List[Gate_ID/2].type!="UNDEF"){
      errGate = &(Gate_List[Gate_ID/2]);
      return parseError(REDEF_GATE);
    }
    //cerr<<i+4<<endl;
    Gate_List[Gate_ID/2].type = "PI";
    Gate_List[Gate_ID/2].Line_No = lineNo+1;
    *(PI_List+i) = Gate_ID/2;
    lineNo++;
    //cerr<<*(PI_List+i)<<endl;
   }
   colNo = 0;
   //Read PO
   for(int i = 0; i < O; i++){
    int Gate_ID;
    if(!getline(cirfile,buffer)){
      errMsg = "PO";
      return parseError(MISSING_DEF);
    }
    //cerr<<i<<endl;
    if(buffer.empty()){
      errMsg = "PO literal ID";
      return parseError(MISSING_NUM);
    }
    if(buffer.find(' ')!=string::npos){
      if(buffer[0] == ' '){
        return parseError(EXTRA_SPACE);
      }else{
        colNo+= buffer.find(' ');
        return parseError(MISSING_NEWLINE);
      }
    }
    //cerr<<i+1<<endl;
    if(buffer.find('\t')!=string::npos){
      if(buffer[0] == '\t'){
        errInt = '\t';
        colNo = buffer.find('\t');
        return parseError(ILLEGAL_WSPACE);
      }else{
        colNo+= buffer.find(' ');
        return parseError(MISSING_NEWLINE);
      }
    }
    //cerr<<i+2<<endl;
    if(!myStr2Int(buffer, Gate_ID) || Gate_ID < 0){
      errMsg = "PO literal ID(" + buffer + ")";
      return parseError(ILLEGAL_NUM);
    }
    if(Gate_ID/2 > M){
      errInt = Gate_ID;
      return parseError(MAX_LIT_ID);
    }
    //cerr<<i+3<<endl;
    if(Gate_List[Gate_ID/2].type == "PO"){
      errGate = &(Gate_List[Gate_ID/2]);
      return parseError(REDEF_GATE);
    }
    //cerr<<i+4<<endl;
    Gate_List[M+i+1].type = "PO";
    Gate_List[M+i+1].Line_No = lineNo+1;
    Gate_List[M+i+1].fanin_1 = Gate_ID;
    Gate_List[Gate_ID/2].fanout_List.push_back(2*(M+i+1)+Gate_ID%2);

    *(PO_List+i) = M+i+1;
    lineNo++;
    //cerr<<*(PO_List+i)<<endl;
   }
   //Read AIG
   space_pos[6]={};
   space_pos[0] = -1;
   spacecount = 0;
   colNo = 0;
   for(int i = 0; i < A; i++){
      int Gate_ID, fanin1, fanin2;
      if(!getline(cirfile,buffer)){
        errMsg = "AIG";
        return parseError(MISSING_DEF);
      }
      //cerr<<i<<endl;
      if(buffer.empty()){
        errMsg = "AIG gate literal ID";
        return parseError(MISSING_NUM);
      }
      for(size_t i = 0;i < 3; i++){
        space_pos[6]={};
        space_pos[0] = -1;
        spacecount = 0;
        colNo = 0;
        space_pos[spacecount+1] = buffer.find(' ') + space_pos[spacecount] + 1;
        spacecount++;
        //cerr<<space_pos[spacecount]<<endl;
        wspace_pos = buffer.find('\t') + space_pos[spacecount-1] + 1;
        if(wspace_pos < space_pos[spacecount] && buffer.find('\t')!=string::npos){
          errInt = '\t';
          colNo = wspace_pos;
          return parseError(ILLEGAL_WSPACE);
        }
        token = buffer.substr(0, space_pos[spacecount]-1-space_pos[spacecount-1]);
        if(i == 0){
          if(token == "")return parseError(EXTRA_SPACE);
            if(!myStr2Int(token, Gate_ID) || Gate_ID < 0){
              errMsg = "AIG gate literal ID(" + token + ")";
              return parseError(ILLEGAL_NUM);
            }
            if(buffer.find(' ') == string::npos){
              colNo = token.length();
              return parseError(MISSING_SPACE);
            }
            if(buffer.find(' ')+1 == buffer.length()){
              colNo = buffer.length();
              errMsg = "AIG input literal ID";
              return parseError(MISSING_NUM);
            }
            if(Gate_ID/2 > M){
              errInt = Gate_ID;
              return parseError(MAX_LIT_ID);
            }
            if(Gate_ID%2 == 1){
              errMsg = "AIG";
              return parseError(CANNOT_INVERTED);
            }
            //cerr<<i+3<<endl;
            if(Gate_List[Gate_ID/2].type!="UNDEF"){
              errGate = &(Gate_List[Gate_ID/2]);
              return parseError(REDEF_GATE);
            }
        }else{
          if(token == ""){
            colNo = space_pos[spacecount];
            return parseError(EXTRA_SPACE);
          }
          if(i == 1){
            if(!myStr2Int(token, fanin1) || fanin1 < 0){
              errMsg = "AIG input literal ID(" + token + ")";
              return parseError(ILLEGAL_NUM);
            }
            if(buffer.find(' ') == string::npos){
              string s = to_string(Gate_ID);
              colNo = 1 + s.length() + token.length();
              return parseError(MISSING_SPACE);
            }
            if(buffer.find(' ')+1 == buffer.length()){
              string s = to_string(Gate_ID);
              colNo = 1 + s.length() + buffer.length();
              errMsg = "AIG input literal ID";
              return parseError(MISSING_NUM);
            }
          }else if(i == 2){
            if(!myStr2Int(token, fanin2) || fanin2 < 0){
              errMsg = "AIG input literal ID(" + token + ")";
              return parseError(ILLEGAL_NUM);
            }
          }
        }
        buffer = buffer.substr(space_pos[spacecount]-space_pos[spacecount-1]);
      }

    
    //cerr<<i+4<<endl;
    Gate_List[Gate_ID/2].type = "AIG";
    Gate_List[Gate_ID/2].Line_No = lineNo+1;
    Gate_List[Gate_ID/2].fanin_1 = fanin1;
    Gate_List[Gate_ID/2].fanin_2 = fanin2;
    Gate_List[fanin1/2].fanout_List.push_back(Gate_ID+fanin1%2);
    Gate_List[fanin2/2].fanout_List.push_back(Gate_ID+fanin2%2);
    //cerr << Gate_ID << "|" << fanin1 << "|" << fanin2 <<endl;
    *(AIG_List+i) = Gate_ID/2;
    lineNo++;
    //cerr<<*(PO_List+i)<<endl;
   }
   bool comment = false;
   while(getline(cirfile, buffer)){
    colNo = 0;
    string name = "";
    string index;
    int idx;
    if(buffer.empty() && !comment){
      errMsg = "symbolic name";
      return parseError(MISSING_IDENTIFIER);
    }
    if(buffer.find(' ')!=string::npos){
      if(buffer[0] == ' '){
        return parseError(EXTRA_SPACE);
      }/*else if(buffer[buffer.find(' ')+1] == ' '){
        return parseError(EXTRA_SPACE);
      }*/
      name = buffer.substr(buffer.find(' ')+1);
    }
    //cerr<<i+1<<endl;
    if(buffer.find('\t')!=string::npos){
        errInt = '\t';
        colNo = buffer.find('\t');
        return parseError(ILLEGAL_WSPACE);
    }
    if(buffer[0] == 'i'){
      index = buffer.substr(1, buffer.find(' ')-1);
      if(!myStr2Int(index, idx) || idx < 0){
        errMsg = "symbol index(" + index + ")";
        return parseError(ILLEGAL_NUM);
      }
      if(name == ""){
        errMsg = "symbolic name";
        return parseError(MISSING_IDENTIFIER);
      }
      if(idx >= I){
        errMsg = "PI index";
        errInt = idx;
        return parseError(NUM_TOO_BIG);
      }
      for(size_t i = 0; i < buffer.length(); i++){
        if(buffer[i] < 0x20){
          colNo = i;
          errInt = buffer[i];
          return parseError(ILLEGAL_SYMBOL_NAME);
        }
      }
      if(Gate_List[*(PI_List+idx)].symbol != ""){
        errMsg = "i";
        errInt = idx;
        return parseError(REDEF_SYMBOLIC_NAME);
      }
      Gate_List[*(PI_List+idx)].symbol = name;
      //cerr<<*(PI_List+idx)<<"|"<<Gate_List[*(PI_List+idx)].symbol<<endl;
    }else if(buffer[0] == 'o'){
      index = buffer.substr(1, buffer.find(' ')-1);
      if(!myStr2Int(index, idx) || idx < 0){
        errMsg = "symbol index(" + index + ")";
        return parseError(ILLEGAL_NUM);
      }
      if(name == ""){
        errMsg = "symbolic name";
        return parseError(MISSING_IDENTIFIER);
      }if(idx >= O){
        errMsg = "PO index";
        errInt = idx;
        return parseError(NUM_TOO_BIG);
      }
      for(size_t i = 0; i < buffer.length(); i++){
        if(buffer[i] < 0x20){
          colNo = i;
          errInt = buffer[i];
          return parseError(ILLEGAL_SYMBOL_NAME);
        }
      }
      if(Gate_List[*(PO_List+idx)].symbol != ""){
        errMsg = "o";
        errInt = idx;
        return parseError(REDEF_SYMBOLIC_NAME);
      }
      Gate_List[*(PO_List+idx)].symbol = name;
      //cerr<<*(PO_List+idx)<<"|"<<Gate_List[*(PO_List+idx)].symbol<<endl;
    }else if(buffer[0] == 'c'){
      if(buffer.length() > 1 && comment == false){
        colNo =1;
        return parseError(MISSING_NEWLINE);
      }
      comment = true;
    }else if(comment == false){
      errMsg = buffer[0];
      return parseError(ILLEGAL_SYMBOL_TYPE);
    }
    lineNo++;
   }
   cirfile.close();
   updateDFS();
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
  cout<<endl;
  cout<<"Circuit Statistics"<< endl <<"=================="<<endl;
  cout<<"  PI   "<< right << setw(9) << I <<endl;
  cout<<"  PO   "<< right << setw(9) << O <<endl;
  cout<<"  AIG  "<< right << setw(9) << A <<endl;
  cout<<"------------------"<<endl;
  cout<<"  Total"<< right << setw(9) << I+O+A <<endl;
}

void
CirMgr::printNetlist() 
{
  cout << endl;
  for(size_t i = 0; i < DFS_List.size(); i++){
    cout << "[" << i << "] " << left << setw(4) << Gate_List[DFS_List[i]].type << DFS_List[i];
    if(Gate_List[DFS_List[i]].type == "PO" || Gate_List[DFS_List[i]].type == "AIG"){
      cout << " ";
      if(Gate_List[Gate_List[DFS_List[i]].fanin_1/2].type == "UNDEF") cout << "*";
      if(Gate_List[DFS_List[i]].fanin_1 % 2 == 1) cout << "!";
      cout << Gate_List[DFS_List[i]].fanin_1/2;
    }
    if(Gate_List[DFS_List[i]].type == "AIG"){
      cout << " ";
      if(Gate_List[Gate_List[DFS_List[i]].fanin_2/2].type == "UNDEF") cout << "*";
      if(Gate_List[DFS_List[i]].fanin_2 % 2 == 1) cout << "!";
      cout << Gate_List[DFS_List[i]].fanin_2/2;
    }
    if(Gate_List[DFS_List[i]].symbol != ""){
      cout << " " << "(" << Gate_List[DFS_List[i]].symbol << ")";
    }
    cout << endl;
  }
}
void
CirMgr::updateDFS(){
  DFS_List.clear();
  for(int i = 0; i < O ; i++){
    DFSAList(Gate_List[M+i+1]);
  }
  for(int i = 0; i< M+1+O; i++){
    Gate_List[i].printed = false;
  }
  for(size_t i = 0; i < DFS_List.size(); i++){
    Gate_List[DFS_List[i]].DFS_NO = i;
  }
}
void
CirMgr::DFSAList(CirGate& output)
{
  if(output.fanin_1!=-1 && Gate_List[output.fanin_1/2].type != "UNDEF" && Gate_List[output.fanin_1/2].printed == false) DFSAList(Gate_List[output.fanin_1/2]);
  if(output.fanin_2!=-1 && Gate_List[output.fanin_2/2].type != "UNDEF" && Gate_List[output.fanin_2/2].printed == false) DFSAList(Gate_List[output.fanin_2/2]);
  if(output.type !="UNDEF" && output.printed == false){
    DFS_List.push_back(&output-Gate_List);
    output.printed = true;
  }
  //cerr << output.type << &output-Gate_List;
}

void
CirMgr::traversal(CirGate& output)
{
  
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(int i = 0; i < I; i++){
    cout << " " << *(PI_List+i);
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(int i = 0; i < O; i++){
    cout << " " << *(PO_List+i);
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
  bool firstfl =false; 
  bool firstundef = false;
  for(int i = 0; i < A ; i++){
    unsigned z = *(AIG_List+i) ;
    int j = Gate_List[z].fanin_1;
    int k = Gate_List[z].fanin_2;
    if(((j == -1)||(k == -1)||(Gate_List[k/2].type == "UNDEF")||(Gate_List[j/2].type == "UNDEF")) && Gate_List[z].type == "AIG"){
      if(!firstfl){
        cout << "Gates with floating fanin(s):" ;
        firstfl = true;
      }
      cout << " " << z;
    }

  }
  for(int i = 0; i < O; i++){
    if(Gate_List[Gate_List[M+i+1].fanin_1/2].type == "UNDEF"){
      if(!firstfl){
        cout << "Gates with floating fanin(s):" ;
        firstfl = true;
      }
      cout << " " << M+1+i;
    }
  }
  if(firstfl) cout << endl;
  for(int i = 1; i < M+1; i++){
    if((Gate_List[i].type != "UNDEF")&&(Gate_List[i].fanout_List.size() == 0)){
      if(!firstundef){
        cout << "Gates defined but not used  :" ;
        firstundef = true;
      }
      cout << " " << i;
    }
  }
  if(firstundef) cout << endl;
}

void
CirMgr::writeAag(ostream& outfile) 
{
  outfile << "aag " << M << " " << I << " " << L << " " << O << " " << A << endl;
  for(int i = 0; i < I; i++){
    outfile << *(PI_List+i) * 2 << endl;
  }
  for(int i = 0; i < O ; i++){
    outfile << Gate_List[M+1+i].fanin_1 << endl;
  }
  size_t i = 0;
  size_t o = 0;
  for(size_t s = 0; s < DFS_List.size(); s++){
    if(Gate_List[DFS_List[s]].type == "AIG"){
      outfile << DFS_List[s]*2 << " " << Gate_List[DFS_List[s]].fanin_1 << " " << Gate_List[DFS_List[s]].fanin_2 << endl;
    }
  }
  for(int j = 0; j < I; j++){
    unsigned z = *(PI_List+j) ;
    if(Gate_List[z].symbol != ""){
      outfile << "i" << i << " " << Gate_List[z].symbol <<endl;
      i++;
    }
  }
  for(int j = 0; j < O; j++){
    if(Gate_List[M+1+j].symbol != ""){
      outfile << "o" << o << " " << Gate_List[M+1+j].symbol <<endl;
      o++;
    }
  }
  for(int i = 0; i< M+1+O; i++){
    Gate_List[i].printed = false;
  }
  outfile << "c" << endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g)
{
  vector<size_t> Pilist;
  vector<size_t> Aiglist;
  vector<size_t> netlist;
  cirwg(*g, Pilist, Aiglist, netlist);
  for(int i = 0; i< M+1+O; i++){
    Gate_List[i].printed = false;
  }
  sort(Pilist.begin(), Pilist.end(), [&](size_t a,size_t b){return (Gate_List[a].Line_No <  Gate_List[b].Line_No  );});
  outfile << "aag " << (g-Gate_List) << " " << Pilist.size() << " " << 0 << " " << "1 " << Aiglist.size() << endl;
  for(size_t i = 0; i < Pilist.size(); i++){
    outfile << Pilist[i]*2 << endl;
  }
  outfile << (g-Gate_List)*2 << endl;
  for(size_t i = 0; i < netlist.size(); i++){
    if(Gate_List[netlist[i]].type == "AIG"){
      outfile << netlist[i]*2 << " ";
      if(Gate_List[Gate_List[netlist[i]].fanin_1/2].type == "UNDEF") outfile << "*";
      outfile << Gate_List[netlist[i]].fanin_1;
      outfile << " ";
      if(Gate_List[Gate_List[netlist[i]].fanin_2/2].type == "UNDEF") outfile << "*";
      outfile << Gate_List[netlist[i]].fanin_2;
      outfile << endl;
    }
  }
  if(g->type == "PO")outfile << (g-Gate_List)*2 << " " << Gate_List[(g-Gate_List)].fanin_1  << endl;
  outfile << "o0 " << (g-Gate_List) << endl << "c" << endl;

}
void
CirMgr::cirwg(CirGate &output, vector<size_t> &Pilist, vector<size_t> &Aiglist, vector<size_t> &netlist)
{
  if(output.fanin_1!=-1 && Gate_List[output.fanin_1/2].type != "UNDEF" && Gate_List[output.fanin_1/2].printed == false) cirwg(Gate_List[output.fanin_1/2],Pilist,Aiglist,netlist);
  if(output.fanin_2!=-1 && Gate_List[output.fanin_2/2].type != "UNDEF" && Gate_List[output.fanin_2/2].printed == false) cirwg(Gate_List[output.fanin_2/2],Pilist,Aiglist,netlist);
  if(output.type !="UNDEF" && output.printed == false){
    netlist.push_back(&output-Gate_List);
    output.printed = true;
    if(output.type == "PI"){Pilist.push_back(&output-Gate_List);}
    else if(output.type == "AIG"){Aiglist.push_back(&output-Gate_List);}
  }
}
void
CirMgr::printFECPairs() 
{
  size_t count = 0;
  //if(Gate_List[DFS_List[0]].fraigsim == 1)return;
  /*for(size_t i = 0; i < FECmap->numBuckets(); i++){
    sort(FECmap->_buckets[i].begin(), FECmap->_buckets[i].end(), Nodecomp);
  }
  for(size_t i = 0; i < FECmap->numBuckets(); i++){
    sort(*(FECmap->_buckets), *(FECmap->_buckets+i), FECcomp);
  }
  //FECmap -> mapsort();
  for(size_t i = 0; i < FECmap->numBuckets(); i++){
    if((!(*(FECmap->_buckets+i)).empty()) && ((*(FECmap->_buckets+i)).size() > 1)){
      cout << "[" << count << "]";
      count++;
      for(size_t j = 0; j < (*(FECmap->_buckets+i)).size(); j++){
        cout << " " ;
        if(j!=0){
          if(Gate_List[(*(FECmap->_buckets+i))[j].second.GateID()].IFEC ^ Gate_List[(*(FECmap->_buckets+i))[0].second.GateID()].IFEC) cout << "!";
        } 
        cout << (*(FECmap->_buckets+i))[j].second.GateID() ;
      }
      cout << endl;
    }
  }*/
  vector< pair<size_t, size_t> > v;

  for(map<size_t, vector<size_t>>::iterator it = FECgrp.begin(); it!=FECgrp.end(); it++){
    if(it->second.size() > 1){
      sort(it->second.begin(), it->second.end());
      v.push_back(make_pair(it->second[0], it->first));
    }
      /*cout << "[" << count << "]";
      count++;
      for(size_t i = 0; i < it->second.size(); i++){
        cout << " ";
        if(i != 0){
          if(Gate_List[it->second[i]].IFEC ^ Gate_List[it->second[0]].IFEC) cout << "!";
        }
        cout << it->second[i];
      }
        cout << endl;
      }*/
  }
  sort(v.begin(),v.end(), [&](pair<size_t, size_t> a, pair<size_t, size_t> b){return (a.first < b.first);});
  for(size_t i = 0; i < v.size(); i++){
    cout << "[" << count << "]";
    count++;
    size_t l = FECgrp[v[i].second].size();
    for(size_t j = 0 ; j < l; j++){
      cout << " ";
      if(Gate_List[FECgrp[v[i].second][j]].IFEC ^ Gate_List[FECgrp[v[i].second][0]].IFEC) cout << "!";
      cout << FECgrp[v[i].second][j];
    }
    cout << endl;
  }

}