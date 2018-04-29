/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"
#include  <cstdlib>

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
  int num,arr_size;
  string token;
  vector<string> tokens;
  size_t n = myStrGetTok(option, token);
  while(token.size()){
    tokens.push_back(token);
    n = myStrGetTok(option, token, n);
  }
  //
  if(tokens.empty())   
    return errorOption(CMD_OPT_MISSING, "");
  else if(tokens.size() >3)
    return errorOption(CMD_OPT_EXTRA, tokens[3]);
  else if(tokens.size()==1)  {                     //push obj
    if(!myStr2Int(tokens[0], num))
      return errorOption(CMD_OPT_ILLEGAL, tokens[0]);
    if(num<=0)
      return errorOption(CMD_OPT_ILLEGAL, std::to_string(num));
    mtest.newObjs(num);
    return CMD_EXEC_DONE;
  }
  for(size_t i=0;i<tokens.size();i++){             //push Arr
    if(myStrNCmp("-Array",tokens[i], 2) ==0 ){
      if(++i>=tokens.size())    {
        errorOption(CMD_OPT_MISSING,"");
        return CMD_EXEC_ERROR;
      }
      if(!myStr2Int(tokens[i], arr_size) )
        return errorOption(CMD_OPT_ILLEGAL, tokens[0]);
      if(arr_size<=0)
        return errorOption(CMD_OPT_ILLEGAL, std::to_string(arr_size));
    }
    else if(myStr2Int(tokens[i], num));
    else{
      return errorOption(CMD_OPT_ILLEGAL, tokens[i]);
    }
  }
  try{
    mtest.newArrs(num,arr_size);
  }
  catch(std::bad_alloc& ba){
    return CMD_EXEC_ERROR;
  }
  return CMD_EXEC_DONE;
}   

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
  int pos,num;
  bool is_array = false, is_index =false, is_random =false;
  vector<int> ran_vec;

  string token;                          //pre-process
  vector<string> tokens;
  size_t n = myStrGetTok(option, token);
  while(token.size()){
    tokens.push_back(token);
    n = myStrGetTok(option, token, n);
  }
  if(tokens.size()<2)
    return errorOption(CMD_OPT_MISSING,"");
  // else if(tokens.size()>3)
  //   return errorOption(CMD_OPT_EXTRA,tokens[3]);
  for(size_t i=0;i<tokens.size();i++){    
    if(myStrNCmp("-Array",tokens[i], 2) ==0 ){
      is_array = true;
    }
    else if (myStrNCmp("-Index",tokens[i], 2) ==0 ){
      if(is_index || is_random)
        return errorOption(CMD_OPT_EXTRA,tokens[i]);
      is_index =true;      i+=1;
      if(!myStr2Int(tokens[i], pos)){
        cerr<<"Error: "<<tokens[i]<<" is not a number"<<endl;
        return CMD_EXEC_ERROR;
      }
    }
    else if(myStrNCmp("-Random",tokens[i], 2) ==0 ){
      if(is_random || is_index)
        return errorOption(CMD_OPT_EXTRA,tokens[i]);
      is_random =true;      i+=1;
      if(!myStr2Int(tokens[i], num))
        return errorOption(CMD_OPT_ILLEGAL,tokens[i]);
      if(num<=0)
        return errorOption(CMD_OPT_ILLEGAL,std::to_string(num));
    }
    else{
      return errorOption(CMD_OPT_ILLEGAL, tokens[i]);
    }
  }
  if(!is_index && !is_random)
    return errorOption(CMD_OPT_MISSING, "");


  if(is_array){             //DELETE ARRAY
    if(mtest.getArrListSize()==0){
      cerr<<"Size of array list is 0!!"<<endl;
      return errorOption(CMD_OPT_ILLEGAL, "-r");
    }
    if(is_random){ 
      for(int i=0;i<num;i++){
        int ran_num = rnGen(mtest.getArrListSize());
        //mtest.deleteArr(ran_num);
        try{
          mtest.deleteArr(ran_num);
        }
        catch(const char* str){
          cerr<<"Error: "<<str<<endl;
        }
      }
    }
    else{
      try{
        mtest.deleteArr(pos);
      }
      catch(const char* str){
        cerr<<"Error: "<<str<<endl;
      }
    } 
  }
  else {                     //DELETE OBJECT
    if(mtest.getObjListSize()==0){
      cerr<<"Size of object list is 0!!"<<endl;
      return errorOption(CMD_OPT_ILLEGAL, "-r");
    }
    if(is_random){
      for(int i=0;i<num;i++){
        int ran_num = rnGen(mtest.getObjListSize());
        mtest.deleteObj(ran_num);
      }
    }
    else{
      try{
        mtest.deleteObj(pos);
      }
      catch(const char* str){
        cerr<<"Error: "<<str<<endl;
      }
    }     
  }
  return CMD_EXEC_DONE;
}   

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


