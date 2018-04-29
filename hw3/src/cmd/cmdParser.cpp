/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"
#include "cmdCommon.h"
#include <string>
#include <vector>

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();

//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   ifstream* tmp =new ifstream(dof.c_str());
   if(!tmp->is_open()){
      if(_dofileStack.size()!= 0)  _dofile = _dofileStack.top();
      else _dofile=0;
      return false;         
   }
   _dofileStack.push(tmp);
   _dofile =tmp;
   return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   // TODO...
   _dofile->close();
   delete _dofile;   
   _dofileStack.pop();
   if(!_dofileStack.empty() ){
      _dofile = _dofileStack.top();
   }
   else _dofile =NULL;
}
// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(cmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0){
         return e->exec(option);
      }
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void CmdParser::printHelps() const
{
   // TODO...
   for( CmdMap::const_iterator it=_cmdMap.begin(); it != _cmdMap.end();++it){
      it->second->help();
   }
   cout<<endl;
}

void CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
   assert(_tempCmdStored == false);
   assert(!_history.empty());
   string str = _history.back();
   assert(str[0] != 0 && str[0] != ' ');
   string first_cmd ;
   size_t end = myStrGetTok(str,first_cmd);
   if(end!= string::npos){
      str =str.substr(end);
      size_t found =str.find_first_not_of(' ');
      str.erase(0,found);
      option = str; 
   }

   CmdExec* e=0;
   if(getCmd(first_cmd) !=0) {
      return getCmd(first_cmd);
   }
   else cerr<<" Illegal command!! ("<<str<<")"<<endl; 
   return e;
}

// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    [Before] if prefix is empty, print all the file names
//    cmd> help $sdfgh
//    [After]
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

void
CmdParser::listCmd(const string& str)
{
   // TODO...
   string cursor_str;
   size_t end = myStrGetTok(str,cursor_str);

   vector<string> dir_str;
   bool check_same =true;
   if(str.find_first_not_of(' ') == string::npos){    //case 1     print-all
      cout<<endl;
      cout << setw(12) << left << "DBAPpend";
      cout << setw(12) << left << "DBAVerage";
      cout << setw(12) << left << "DBCount";
      cout << setw(12) << left << "DBDelete";
      cout << setw(12) << left << "DBMAx" <<endl;
      cout << setw(12) << left << "DBMIn";
      cout << setw(12) << left << "DBPrint";
      cout << setw(12) << left << "DBRead";
      cout << setw(12) << left << "DBSOrt";
      cout << setw(12) << left << "DBSUm" <<endl;
      cout << setw(12) << left << "DOfile";     
      cout << setw(12) << left << "HELp";
      cout << setw(12) << left << "HIStory";
      cout << setw(12) << left << "Quit" ;
      reprintCmd();    
   }

   else if(getCmd(cursor_str)!=0 )  {        
      if(str.find(' ') != string::npos && _tabPressCount > 1){              //case 6 print directory             
         string prefix = str.substr(end+1);
         size_t check_size =prefix.size();
         listDir(dir_str,prefix,".");
         if(!dir_str.empty() ){
            while(check_same){
               check_size+=1;
               for(size_t i=0;i<dir_str.size();i++){
                  string comp= dir_str[i].substr(0,check_size);
                  if(myStrNCmp(dir_str[0],comp,check_size != 0)){
                     check_size-=1;
                     check_same =false;
                     break;
                  }
               }
               if(check_size == dir_str[0].size()) check_same =false;
            }
            for(size_t i=prefix.size();i<check_size;i++){
                insertChar(dir_str[0][i]);
            }
            if(dir_str.size()==1)  insertChar(' ');
            if(check_size ==prefix.size()){
               for(size_t i=0;i<dir_str.size();i++){
                  if(i%5==0) cout<<endl;
                  cout << setw(16) << left << dir_str[i];
               }
            reprintCmd(); 
            }   
         }   
         else mybeep();       
      }
      else if(str.find(' ') != string::npos && _tabPressCount <= 1){          //case 5 already match print usage
            cout<<endl;                   
            getCmd(cursor_str)->usage(cout);
            _tabPressCount=1;
            reprintCmd(); 
         }  
      else {                                                                   //case 3 exactly match and print
         string correct_str;
         correct_str =getCmd(cursor_str)->getOptCmd();
         for(size_t i=cursor_str.size() ;i< correct_str.size() ;i++)    
            insertChar(correct_str[i]);
            insertChar(' ');
            _tabPressCount=0;
      }
   }
  
   else{                                                                 //case 4 no match  &  case 2 muti-match       
      vector<string> storing;
      bool checking = false;   
      if(cursor_str.size()<4){
         for(CmdMap::iterator it=_cmdMap.begin(); it != _cmdMap.end(); ++it){    
            if(myStrNCmp(it->second->getOptCmd(),cursor_str,cursor_str.size())==0 ){
               checking =true;
               storing.push_back(it->second->getOptCmd());
            } 
         }
      }
      if(checking){
         if(storing.size()==1){                                       //case 3 exactly match and print
            for(size_t i=cursor_str.size();i<storing[0].size();i++)
               insertChar(storing[0][i]);
               insertChar(' ');
         }
         else{
            for(size_t i=0;i<storing.size();i++){
               if(i%5==0)cout<<endl;
               cout << setw(12) << left <<storing[i];
            }
            reprintCmd();
         }
         
      }
      else   mybeep();
   } 
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command   in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
//    ==> Checked by the CmdExec::checkOptCmd(const string&) function
// 3. All string comparison are "case-insensitive".
//
CmdExec* CmdParser::getCmd(string cmd)
{
   CmdExec* e = 0;
   for(CmdMap::iterator it=_cmdMap.begin(); it != _cmdMap.end(); ++it){
      if(myStrNCmp(it->second->getOptCmd(),cmd,it->first.size())==0 ){
      e = it->second;
      }
   }
   // TODO...
   return e;
}

//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

