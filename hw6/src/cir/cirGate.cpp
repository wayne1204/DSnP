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

extern CirMgr *cirMgr;
unsigned CirGate::_globalVisit = 0;
// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
string CirGate::getTypeStr() const { 
  	switch (_gatetype){
  		case 0: 
  			return "UNDEF";
  			break;
  		case 1:
  			return "PI";
  			break;
  		case 2:
  			return "PO";
  			break;
  		case 3:
  			return "AIG";
  			break;
  		case 4:
  			return "CONST";
  			break;
  		default:
  			break;
    }
    return "";
}

void CirGate::printGate() const{
	switch (_gatetype){
  		case PI_GATE:
  			cout<<"PI  "<<_gid;
  			if(_gatename != "") cout<<" ("<<_gatename<<")";
  			break;
  		case PO_GATE:
  			cout<<"PO  "<<_gid<<" ";
  			if(getFaninA()->getTypeStr() == "UNDEF") cout<<"*";
  			if(_is_a_neg) cout<<"!";
  			cout<<getFaninA()->getID();
  			if(_gatename != "") cout<<" ("<<_gatename<<")";
  			break;
  		case AIG_GATE:
  			cout<<"AIG "<<_gid<<" ";
  			if(getFaninA()->getTypeStr() == "UNDEF") cout<<"*";
  			if(_is_a_neg) cout<<"!";
  			cout<<getFaninA()->getID()<<" ";
  			if(getFaninB()->getTypeStr() == "UNDEF") cout<<"*";
  			if(_is_b_neg) cout<<"!"; 
  			cout<<getFaninB()->getID();
  			break;
  		case CONST_GATE:
  			cout<<"CONST0";
  			break;
    }
    cout<<endl;
}

void CirGate::writeGate(stringstream& outfile) const{
	switch (_gatetype){
		case PI_GATE:
			outfile<<getID()*2<<endl;
			break;
		case PO_GATE:
			if(isVisit()) return;
			if(_is_a_neg) outfile<<getFaninA()->getID()*2+1<<endl;
			else outfile<<getFaninA()->getID()*2<<endl;
			break;
		case AIG_GATE:
			outfile<<_gid*2<<" ";
			if(_is_a_neg) outfile<<getFaninA()->getID()*2+1<<" ";
			else outfile<<getFaninA()->getID()*2<<" ";
			if(_is_b_neg) outfile<<getFaninB()->getID()*2+1<<endl;
			else outfile<<getFaninB()->getID()*2<<endl;
			break;
	}
}

void CirGate::reportGate() const{
	stringstream ss;
	cout<<"==================================================\n";
	ss<<getTypeStr()<<"("<<getID()<<")";
	if(_gatename != "") ss<<"\""<<_gatename<<"\"";
	ss<<", line "<<getLineNo();
	cout<<"= "<<setw(47)<<left<<ss.str()<<"=\n";
	cout<<"==================================================\n";
}

void CirGate::reportFanin(int level) const {
    assert (level >= 0);
    CirGate::setGlobalVisit();
    printFanin(level, 2);
}

void CirGate::printFanin(int level, size_t indent) const {
   	cout<<getTypeStr()<<" "<<getID();
   	if(isVisit() && level > 0) {
		cout<<" (*)\n";
		return;
	}
	cout<<endl;
   	if(fan_in_a != 0 && level > 0){  	
	   setVisit();
       cout<<string(indent, ' ');
   	   if(_is_a_neg) cout<<"!";
   	   getFaninA()->printFanin(level -1, indent+2);
   	}
   	if(fan_in_b != 0 && level > 0){
   	   setVisit();
   	   cout<<string(indent, ' ');
   	   if(_is_b_neg) cout<<"!";
   	   getFaninB()->printFanin(level -1, indent+2);
   	} 
}

void CirGate::reportFanout(int level) const{
    assert (level >= 0);
    CirGate::setGlobalVisit();
    printFanout(level, 2);
}

void CirGate::printFanout(int level, size_t indent) const{
	cout<<getTypeStr()<<" "<<getID();
	if(isVisit() && level > 0) {
		cout<<" (*)\n";
		return;
	}
	cout<<endl;
	if(level > 0){
		for(size_t i=0; i<fan_out.size(); ++i){
			setVisit();
			cout<<string(indent, ' ');
			if(fan_out_neg[i]) cout<<"!";
			fan_out[i]->printFanout(level -1, indent+2);
		}
	}
}