/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cassert>
#include <stdexcept>  
#include <cstring>
#include <cstdlib>
#include <ctype.h>
#include <fstream>
#include <sstream>
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
// static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

stringstream subfile;
int calltimes;
int counter;

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
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
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
  string line, token;
  size_t n;
  unsigned lit;
  ifstream ifs(fileName.c_str(),ios::in);
  if(!ifs.is_open() ) {
    cerr << "Cannot open design \""<<fileName<<"\"!!\n";
    return false;
  } 
  // read aag
  getline(ifs,line); lineNo = -1;
  if (!checkWspace(line)) return false;
  if (line.size() == 3) {
    errMsg = "number of variables";
    return parseError(MISSING_NUM);
  }
  myStrGetTok(line, errMsg);
  if(errMsg == "aag"){
    line = line.substr(4);  colNo=4;
    if (line == "") {
      errMsg = "number of variables";
      return parseError(MISSING_NUM);
    }
    if (!cutline(line, max_num))   return false;
    if (!cutline(line, PI_num))    return false;
    if (!cutline(line, FF_num))    return false;
    if (!cutline(line, PO_num))    return false;
    if (!cutline(line, AIG_num))   return false;
    if (PI_num+AIG_num >  max_num){
      errInt = (int)max_num;
      errMsg = "Number of variables";
      return parseError(NUM_TOO_SMALL);
    }
  }
  else if(line[0] == ' ')
    return parseError(EXTRA_SPACE);
  else
    return parseError(ILLEGAL_IDENTIFIER);

  // read PI
  _gatelist = new CirGate* [max_num + PO_num +1] ();
  for(unsigned i =0;i< PI_num; ++i){
    if(!getline(ifs,line)) return parseError(MISSING_NEWLINE);
    lineNo++; colNo=0;
    // if( !checkWspace(line)) return false;
    if( !cutline(line, lit)) return false;
    if( !RegisterGid(lit) )    return false;
    CirGate* c = new CirGate(lit/2, lineNo+1, PI_GATE);
    _gatelist[lit/2] = c;
    PI_list.push_back(c);
  }

  for(unsigned i =0; i< PO_num; ++i){
    if(!getline(ifs,line)) return parseError(MISSING_NEWLINE);
    lineNo++; colNo=0;
    // if( !checkWspace(line)) return false;
    if( !cutline(line, lit)) return false;
    PA_list.push_back(lit);

    CirGate* output = new CirGate(max_num + 1 + i, lineNo+1, PO_GATE);
    PO_list.push_back(output);
    _gatelist[ max_num + 1 + i ] = output;
  }
  for(unsigned i=0; i<AIG_num; ++i){
    if(!getline(ifs,line)) return parseError(MISSING_NEWLINE);
    lineNo++; colNo=0;  
    // if( !checkWspace(line)) return false;
    if( !cutline(line, lit)) return false;
    if( !RegisterGid(lit))  return false;
    CirGate* gate = new CirGate(lit/2, lineNo+1, AIG_GATE);
    _gatelist[lit/2] = gate;

    AIG_list.push_back(lit/2);
    if( !cutline(line, lit)) return false;
    AIG_list.push_back(lit);
    if( !cutline(line, lit)) return false;
    AIG_list.push_back(lit);
  }
  if( !initGates())  return false;

  lineNo = PI_num+PO_num+AIG_num;
  //parsing symbol line
  while(getline(ifs,line)){ 
    if(line == "c") break;
    if(!checkWspace(line))  return false;
    if(myStrNCmp(line,"c",1) == 0 && line.size() == 1)
      return parseError (MISSING_NEWLINE);
    n = myStrGetTok(line, token);  
    
    if(myStrNCmp(token,"i",1) == 0){
      token = token.substr(1);
      if(!myStr2Int(token, errInt)){
        errMsg = token;
        return parseError(ILLEGAL_SYMBOL_TYPE);
      } 
      if(errInt >= (int)PI_num){
        errMsg = "PI index"; 
        return parseError(NUM_TOO_BIG);
      }
      myStrGetTok(line, token, n);
      if(PI_list[errInt]->getGateName() != ""){
        errMsg ="i";
        return parseError(REDEF_SYMBOLIC_NAME);
      }
      else if(token == "") {
        errMsg = "\"symbolic name\"";
        return parseError(MISSING_NUM);
      }
      PI_list[errInt]->setGateName(token);
    }
    else if(myStrNCmp(token,"o",1) == 0){
      token = token.substr(1);
      if(!myStr2Int(token, errInt)) {
        errMsg = token;
        return parseError(ILLEGAL_SYMBOL_TYPE);
      }
      if(errInt >= (int)PO_num){
        errMsg = "PO index"; 
        return parseError(NUM_TOO_BIG);
      }
      myStrGetTok(line, token, n);
      if(PO_list[errInt]->getGateName() != ""){
        errMsg ="o"; 
        return parseError(REDEF_SYMBOLIC_NAME);
      }
      else if(token == "") {
        errMsg = "\"symbolic name\"";
        return parseError(MISSING_NUM);
      }
      PO_list[errInt]->setGateName(token);
    }
    else {
      errMsg = token;
      return parseError(ILLEGAL_SYMBOL_TYPE);
    }
  }
  return true;
}
//  checking Wspace, extra space, missing new line update line&col simultaneously
bool CirMgr::checkWspace(string& line){  
  if(line == "")    return parseError(MISSING_NEWLINE);
  lineNo++; colNo=0;
  if(line[0] == ' ')  return parseError(EXTRA_SPACE);
  for(size_t i=0; i<line.size(); ++i){
    if( isspace(line[i]) && line[i] != ' '){
      errInt = line[i];   colNo= i;
      return parseError(ILLEGAL_WSPACE);
    }
    if(!isprint(line[i]) ){
      errInt = line[i];
      return parseError(ILLEGAL_SYMBOL_NAME);
    }
  }
  return true;
}

//   cut string to token and transform to unsigned
bool CirMgr::cutline(string& line, unsigned& number){
  if (line[0] == ' ') return parseError(EXTRA_SPACE);
  size_t pos = line.find_first_of(' ');
  string token = line.substr(0, pos);
  try{
    number = stoul(line);
  }catch(std::invalid_argument){
    errMsg = line;
    return parseError(ILLEGAL_NUM);
  }
  line = line.substr(pos+1);
  colNo += (pos+1);
  return true;
}

bool CirMgr::initGates(){
  CirGate* c = new CirGate(0, 0, CONST_GATE);
  _gatelist[0] = c;
  lineNo = PI_num+PO_num;
  
  for(unsigned i=0; i<AIG_num; ++i){     // link AIG gates
    unsigned gid =AIG_list[i*3];  lineNo++;
    if(!setLink(_gatelist[gid], AIG_list[i*3+1])) return false;
    if(!setLink(_gatelist[gid], AIG_list[i*3+2])) return false;
  }
  lineNo = PI_num;
  for(unsigned j=0; j<PO_num; ++j) {          //connect PO & AIG gates
    lineNo++;
    if(!setLink(_gatelist[ max_num + 1 +j ], PA_list[j]) ) return false;
  }
  for(unsigned i=1; i<max_num+1; ++i){
    if(getGate(i) != 0){
      if( !_gatelist[i]->havefanout())
        FL_nu_list.push_back((unsigned)i);
    }   
  }
  return true;
}

bool CirMgr::setLink(CirGate* cur_gate, unsigned& literal){
  bool negation;
  if( !checkGid(literal, negation)) return false;
  if(getGate(literal/2) == NULL || getGate(literal/2)->getTypeStr() == "UNDEF"){
    CirGate* newgate = new CirGate(literal/2,0, UNDEF_GATE);
    _gatelist[literal/2] = newgate;
    if(cur_gate->getFaninA() == NULL) 
      FL_in_list.push_back(cur_gate->getID());
    else if(cur_gate->getFaninA()->getTypeStr() != "UNDEF") 
      FL_in_list.push_back(cur_gate->getID());
  }
  _gatelist[literal/2]->addFanout(cur_gate, negation);
  if(cur_gate->getFaninA() == 0)
    cur_gate->setFaninA(_gatelist[literal/2], negation);
  else
    cur_gate->setFaninB(_gatelist[literal/2], negation);
  return true;
}
// pass literal and check whether it's legal
bool CirMgr::RegisterGid(unsigned& literal){
  errInt = (int) literal;
  if(literal/2 > max_num)
    return parseError(MAX_LIT_ID);
  if(literal/2 == 0 )
    return parseError(REDEF_CONST);  // AIG -2 -3  PO
  if(literal % 2 != 0){
    if(lineNo <= (unsigned)PI_num) errMsg ="PI";
    else  errMsg = "AIG";
    return parseError(CANNOT_INVERTED);
  }
  if (getGate(literal/2) != 0){
    errGate = getGate(literal/2);
    return parseError(REDEF_GATE);  //  AIG -2 -3
  }
  return true;
}
// pass literal and check whether it's legal
bool CirMgr::checkGid(unsigned& literal, bool& neg){
  // if( token == "" ){
  //   if(lineNo <= (unsigned)PI_num) errMsg ="PI";
  //   else if(lineNo <= (unsigned)PI_num+PO_num ) errMsg = "PO";
  //   else errMsg = "AIG";
  //   return parseError(MISSING_NUM);
  // }
  errInt = (int)literal;
  if(literal/2 > max_num)
    return parseError(MAX_LIT_ID);
  neg = errInt % 2;
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
void CirMgr::printSummary() const{
  cout<<endl;
  cout<<"Circuit Statistics"<<endl;
  cout<<"=================="<<endl;
  cout<<"  PI  "<<setw(10)<<right<<PI_num<<endl;
  cout<<"  PO  "<<setw(10)<<right<<PO_num<<endl;
  cout<<"  AIG "<<setw(10)<<right<<AIG_num<<endl;
  cout<<"------------------"<<endl;
  cout<<"  Total"<<setw(9)<<right<<PI_num+PO_num+AIG_num<<endl;

}

void
CirMgr::printPIs() const
{
  cout << "PIs of the circuit:";
  for(unsigned i=0; i<PI_num; ++i){
    cout<<" "<<PI_list[i]->getID();
  }
  cout << endl;
}

void
CirMgr::printPOs() const
{
  cout << "POs of the circuit:";
  for(unsigned i=0; i<PO_num; ++i){
    cout<<" "<<PO_list[i]->getID();
  }
  cout << endl;
}

void CirMgr::printFloatGates() const{
  if(!FL_in_list.empty()){
    cout<<"Gates with floating fanin(s):";
    for(size_t i=0; i<FL_in_list.size(); ++i)  cout<<" "<<FL_in_list[i];
      cout<<endl;
  }
  if(!FL_nu_list.empty()){
    cout<<"Gates defined but not used  :";
    for(size_t i=0; i<FL_nu_list.size(); ++i)  cout<<" "<<FL_nu_list[i];
      cout<<endl;
  }
}

void CirMgr::printNetlist() const{ 
  calltimes = 0;
  CirGate::setGlobalVisit();
  cout<<endl;
  for(unsigned i=0; i<PO_num; ++i){
    cirDFS(PO_list[i]);
  }
}

void CirMgr::cirDFS(CirGate* g) const{
  if(g->isVisit() || g->getTypeStr() == "UNDEF")  return;
  g->setVisit();
  if(g->getFaninA() != NULL )
    cirDFS(g->getFaninA());
  if(g->getFaninB() != NULL )
    cirDFS(g->getFaninB());
  cout<<"["<<calltimes<<"] ";
  g->printGate();
  calltimes++;
}

void CirMgr::writeAag(ostream& outfile) const{  
  subfile.str("");
  counter = 0;
  CirGate::setGlobalVisit();
  for(unsigned i=0; i<PI_num; ++i){
    PI_list[i]->writeGate(subfile);
    PI_list[i]->setVisit();
  }
  for(unsigned i=0; i<PO_num; ++i)
    PO_list[i]->writeGate(subfile);
  for(unsigned i=0; i<PO_num; ++i){
    writeDFS(subfile, PO_list[i]);
  }
  for(unsigned i=0; i<PI_num; ++i){    //may have null gate name
    if(PI_list[i]->getGateName() != "")
    subfile<<"i"<<i<<" "<<PI_list[i]->getGateName()<<endl;
  }
  for(unsigned j=0;j<PO_num; ++j){
    if(PO_list[j]->getGateName() != "")
    subfile<<"o"<<j<<" "<<PO_list[j]->getGateName()<<endl;
  }
  subfile<<"c\n";
  subfile<<"AAG output by Ping-Wei Huang\n";
  outfile<<"aag "<<max_num<<" "<<PI_num<<" 0 "<<PO_num<<" "
  <<counter<<endl;
  outfile<<subfile.str();
}

void CirMgr::writeDFS(stringstream& file, CirGate* g) const{
  if(g->isVisit())  return;
  g->setVisit();
  if(g->getFaninA() != NULL)
    writeDFS(file, g->getFaninA());
  if(g->getFaninB() != NULL)
    writeDFS(file, g->getFaninB());
  g->writeGate(file);
  if(g->getTypeStr() == "AIG")counter++;
}
