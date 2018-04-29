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

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
// friend class CirMgr;
public:
   CirGate() {}
   CirGate(unsigned n, unsigned l, int type)
    :_gid(n), _lineNo(l), _gatetype(type){
      _visit = 0;
      fan_in_a = NULL;
      fan_in_b = NULL;
   }
   ~CirGate() {}

   // Basic access methods
   unsigned getID() const { return _gid;}
   int getLineNo() const { return _lineNo; }
   string getTypeStr() const;
   string getGateName()const { return _gatename;}
   void setGateName(string& s) {
      _gatename = s; 
    }

   // Printing functions
   void printGate() const;
   void reportGate() const;
   void reportFanin(int level) const;
   void printFanin(int level, size_t indent) const;
   void reportFanout(int level) const;
   void printFanout(int level, size_t indent) const;
   void writeGate(stringstream& outfile) const;

   CirGate* getFaninA()const{ return fan_in_a;}
   void setFaninA(CirGate* g, bool b) { 
     fan_in_a = g;    _is_a_neg = b;
   }
   CirGate* getFaninB()const{ return fan_in_b;}
   void setFaninB(CirGate* g, bool b) { 
     fan_in_b = g;    _is_b_neg = b;
   } 
   bool havefanout()  {return !fan_out.empty(); }
   void addFanout(CirGate* g, bool b)  {
    fan_out.push_back(g); 
    fan_out_neg.push_back(b);
   } 

   static void setGlobalVisit() {_globalVisit++; }
   void setVisit() const { _visit = _globalVisit; }
   bool isVisit() const { return _visit == _globalVisit; }

private:
  //links to other gates
  GateList fan_out;
  vector<bool> fan_out_neg;
  CirGate* fan_in_a;
  CirGate* fan_in_b;

  //gate info
  string _gatename;
  unsigned _gid;
  int _lineNo;
  int _gatetype;

  //for searching
  static  unsigned _globalVisit;
  mutable unsigned _visit;

  bool _is_a_neg;
  bool _is_b_neg;
};

#endif // CIR_GATE_H
