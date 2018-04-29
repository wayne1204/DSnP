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
#include <string>
#include <fstream>
#include <iostream>
#include "cirGate.h"
#include "cirDef.h"
using namespace std;

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(){}
   ~CirMgr() {}

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
    if(gid > max_num+PO_num) return 0;
    if(_gatelist[gid] == 0 ) return 0;
    else return _gatelist[gid];
  }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   bool cutline(string&, unsigned&);
   bool checkWspace(string&);
   bool initGates();
   bool setLink(CirGate*, unsigned&);
   bool RegisterGid(unsigned&);
   bool checkGid(unsigned&, bool&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printNetlist() const;
   void cirDFS(CirGate*)const;
   void writeAag(ostream&) const;
   void writeDFS(stringstream&, CirGate* )const;
   
   
private:
  CirGate** _gatelist;
  GateList PI_list;
  GateList PO_list;
  IdList FL_in_list;
  IdList FL_nu_list;
  IdList PA_list;
  IdList AIG_list;

  unsigned max_num;
  unsigned PI_num;
  unsigned FF_num;
  unsigned PO_num;
  unsigned AIG_num; 
};

#endif // CIR_MGR_H
