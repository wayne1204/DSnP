/****************************************************************************
  FileName     [ dbTable.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Table member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <string>
#include <cctype>
#include <cassert>
#include <set>
#include <algorithm>
#include <climits>
#include <cmath>
#include "dbTable.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBRow& r)
{
   // TODO: to print out a row.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
    for(size_t i=0;i<r.size();i++){
		if(r[i] != INT_MAX ) cout<<r[i];
		else cout<<'.';
		if(i != r.size()-1 )cout<<' ';
	}
	cout<<endl;	
   return os;
}

ostream& operator << (ostream& os, const DBTable& t)
{
   // TODO: to print out a table
   // - Data are seperated by setw(6) and aligned right.
   // - Null cells should be left blank (printed as '.').
	for(size_t i=0;i<t.nRows();i++){
		for(size_t j=0;j<t.nCols();j++){
			if(t[i][j] != INT_MAX ) cout<<std::right<<setw(6)<<t[i][j];
			else cout<<std::right<<setw(6)<<'.';
		}
		cout<<endl;	
	}
	cout<<endl;
   return os;
}

ifstream& operator >> (ifstream& ifs, DBTable& t)
{
   // TODO: to read in data from csv file and store them in a table
   // - You can assume all the data of the table are in a single line.
   string line;
   int value,pos;
   while(getline(ifs,line)){
   		int cr_count=0,comma_count=0;
   		if(line[line.size()-1] ==13 && line[line.size()-2]==13)
   			line.resize(line.size()-1);
	   	for(size_t i=0;i<line.size();i++){
	   	  	if(line[i] == 13 || line[i] == '\n')    cr_count++;
	   	  	if(line[i] == ',')   comma_count++;
	   	}
		//int row_size =cr_count;
		int col_size =comma_count  +1;
			DBRow row;
			for(int j=0;j<col_size;j++){
				if(j==col_size-1) pos =line.find_first_of(13);
				else 			  pos =line.find_first_of(',');
				string tmp =line.substr(0,pos);
				if(!tmp.empty()) value = atoi(tmp.c_str());
				else             value = INT_MAX;
				row.addData(value);
				line.erase(0,pos+1);
			}
		    t.addRow(row);		
   }
   return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void DBRow::removeCell(size_t c)
{
   // TODO
	_data.erase(_data.begin()+c);
}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool
DBSort::operator() (const DBRow& r1, const DBRow& r2) const
{
   // TODO: called as a functional object that compares the data in r1 and r2
   //       based on the order defined in _sortOrder
	for(size_t i=0;i<_sortOrder.size();i++){
		if(r1[_sortOrder[i] ] < r2[_sortOrder[i] ]) return true;
		else if(r1[_sortOrder[i] ] > r2[_sortOrder[i] ])	return false;
	}
   return false;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void DBTable::reset()
{
   // TODO
	_table.erase(_table.begin(), _table.end());
}

void DBTable::addCol(const vector<int>& d)
{
   // TODO: add a column to the right of the table. Data are in 'd'.
	for (size_t i = 0; i < nRows(); ++i)
		_table[i].addData( d[i] );
}

void DBTable::delRow(int c)
{
   // TODO: delete row #c. Note #0 is the first row.
	_table.erase(_table.begin()+c);
}

void DBTable::delCol(int c)
{
   // delete col #c. Note #0 is the first row.
   for (size_t i = 0, n = _table.size(); i < n; ++i)
      _table[i].removeCell(c);
}

// For the following getXXX() functions...  (except for getCount())
// - Ignore null cells
// - If all the cells in column #c are null, return NAN
// - Return "float" because NAN is a float.
float DBTable::getMax(size_t c) const
{
   // TODO: get the max data in column #c
	float max_num = INT_MIN;
	for(size_t  i =0;i<nRows();i++){
		if(_table[i][c] == INT_MAX) continue;
		if( _table[i][c] > max_num)  max_num  = _table[i][c];
	}
	if(max_num != INT_MIN) return max_num;
	else return NAN;
}

float DBTable::getMin(size_t c) const
{
   // TODO: get the min data in column #c
   float min_num = INT_MAX;
	for(size_t  i =0;i<nRows();i++){
		if(_table[i][c] == INT_MAX) continue;
		if( _table[i][c] < min_num)  min_num  = _table[i][c];
	}
	if(min_num != INT_MAX) return min_num;
	else return NAN;	
}

float DBTable::getSum(size_t c) const
{
   // TODO: compute the sum of data in column #c
	float sum=0;
	int counter =0;
	for(size_t i =0;i<nRows();i++){
		if(_table[i][c]!= INT_MAX) { sum += _table[i][c]; counter++;}
	}
   if(counter!=0) return sum;
   else return NAN;
}

int DBTable::getCount(size_t c) const
{
   // TODO: compute the number of distinct data in column #c
   // - Ignore null cells
    vector<bool> storing;
	int count_dif=0;
	for(size_t j=0;j<nRows();j++){
		if(_table[j][c]==INT_MAX) storing.push_back(false);
		else storing.push_back(true);
		for(size_t k=0;k<j;k++){
			if(_table[j][c] ==_table[k][c]) storing[j]=false;
		}
	}
	for(size_t a=0;a<storing.size();a++){
		if(storing[a]==true) count_dif++;
	}
   return count_dif;
}

float DBTable::getAve(size_t c) const
{
   // TODO: compute the average of data in column #c
   	float sum=0;
	int counter =0;
	for(size_t i =0;i<nRows();i++){
		if(_table[i][c]!= INT_MAX) { sum +=  _table[i][c]; counter++;}
	}
   if(counter!=0) return sum/(float)counter;
   else return NAN;
}

void DBTable::sort(const struct DBSort& s)
{
   // TODO: sort the data according to the order of columns in 's'
	std::sort(_table.begin(), _table.end(),s);
}

void DBTable::printCol(size_t c) const
{
   // TODO: to print out a column.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
	for(size_t i=0;i<nRows();i++){
		if(_table[i][c] != INT_MAX ) cout<<_table[i][c];
		else cout<<'.';
		if(i != nRows()-1 )cout<<' ';
	}
	cout<<endl;
}

void DBTable::printSummary() const
{
   size_t nr = nRows(), nc = nCols(), nv = 0;
   for (size_t i = 0; i < nr; ++i)
      for (size_t j = 0; j < nc; ++j)
         if (_table[i][j] != INT_MAX) ++nv;
   cout << "(#rows, #cols, #data) = (" << nr << ", " << nc << ", "
        << nv << ")" << endl;
}

