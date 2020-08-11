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
	_dofileStack.push(_dofile);
    _dofile = new ifstream(dof.c_str());
    if(_dofileStack.size() >= 1024) { return false; }
    if(!_dofile->is_open())
    {
    	closeDofile();
    	return false;
    }
    return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   // TODO...
   delete _dofile;
   _dofile = _dofileStack.top();
   _dofileStack.pop();
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
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
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
      if (e != 0)
         return e->exec(option);
   }

   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
  // TODO...
	for(auto it=_cmdMap.begin(); it!=_cmdMap.end(); ++it)
  { 
		it->second->help();
	}
	cout << endl;
}

void
CmdParser::printHistory(int nPrint) const
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
  // TODO...done
  assert(str[0] != 0 && str[0] != ' ');

  // Get the first token and the end-Idx of the token
  string temp;
  int end = myStrGetTok(str, temp);

  // Make sure the command matches
  // If matches, erase the command part of the str(entire input)
 	CmdExec* e = getCmd(temp);
 	if(e == 0) { cerr << "Illegal command!! (" << temp << ")" << endl; }
 	else { str.erase(0, end); option = str; return e; }

 	return NULL;
}

// Remove this function for TODO...
//
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
//    Considering the following cases in which prefix is empty:
//    --- 6.1.1 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and they do not have a common prefix,
//    cmd> help $sdfgh
//    [After] print all the file names
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.1.2 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and all of them have a common prefix,
//    cmd> help $orld
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help mydb-$orld
//    --- 6.1.3 ---
//    [Before] if prefix is empty, and only one file in the current directory
//    cmd> help $ydb
//    [After] print out the single file name followed by a ' '
//    cmd> help mydb $
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
	bool _exec = true;
	int len = str.length();
  string command_1, command_2;
	for(int i=0;i<len;i++) { if(str[i] != ' ') _exec = false; }

  // If the command is "" or consist of ' 's only print every command
	if(_exec || str == "")
	{
    cout << endl;
		int cnt = 1;
		for(map<string, CmdExec*>::iterator it=_cmdMap.begin(); it!=_cmdMap.end(); ++cnt, ++it)
		{
      command_1 = it->first;
      command_2 = it->second->getOptCmd();
      command_1 += command_2;
			cout << setw(12) << left << command_1;
			if(cnt%5 == 0) { cout << endl; }
		}
    reprintCmd();
	}
  
  // If the command consist of more than spaces
  else
  {
    // Get the first word of the entire input
    string firstWord = "";
    myStrGetTok(str, firstWord);

    // Get the word right before cursor
    string foreTab = "";
    int startIdx = _readBufPtr-_readBuf;
    for(char* i=_readBufPtr-1;i>=_readBuf;startIdx--, i--) { if(*i==' ') break; }
    for(int i=startIdx;i<_readBufPtr-_readBuf;i++) { foreTab += _readBuf[i]; }

    // Determine match or not
    int match, match_2;
    bool beep = true;
    vector<string> matchCmd;

    // If the cursor is not "on" the first word => use firstword for matching    
    int len = firstWord.length();
    if(_readBufPtr-_readBuf > len)
    {
      for(map<string, CmdExec*>::iterator it=_cmdMap.begin(); it!=_cmdMap.end(); ++it)
      {
        // Store the full command in command_1
        command_1 = it->first;
        command_2 = it->second->getOptCmd();
        command_1 += command_2;

        // Fully matched or only the upper-case part neither will do
        match = myStrNCmp(it->first, firstWord, it->first.length());
        match_2 = myStrNCmp(command_1, firstWord, command_1.length());   

        if(match == 0 || match_2 == 0)
        {
          // Although matchCmd is empty, but no beep needed
          beep = false;

          // If tab is pressed for the first time, print the usage of the matched command
          if(_tabPressCount <= 1)
          {
            cout << endl;
            it->second->usage(cout);
            reprintCmd();
          }
          
          // If tab is pressed mutiple times, print the files in the same dir
          else
          {
            vector<string> files;
            string prefix = "";
            bool add = true;

            // If there's nothing before cursor
            if(foreTab == "")
            {        
              listDir(files, "", ".");

              // Get the comman prefix if any
              for(int i=0, l=files[0].length();i<l;i++)
              {
                for(int j=0, s=files.size();j<s;j++)
                {
                  if(files[0][i] != files[j][i]) { add = false; break; }
                }
                if(add) { prefix += files[0][i]; }
              }

              // If there's no common prefix, print every file
              if(prefix == "")
              {
                for(int i=0, s=files.size();i<s;i++)
                {
                  if(i%5 == 0) { cout << endl; }
                  cout << setw(16) << left << files[i];
                }
                reprintCmd();
              }

              // If there's a common prefix, print it out
              else { for(int i=0, l=prefix.length();i<l;i++) insertChar(prefix[i]); } 
            }

            // If want to search for certain prefix(words before cursor)
            else
            {
              listDir(files, foreTab, ".");

              // If there's only one file, print the entire name of it
              if(files.size() == 1 )
              {
                for(int i=foreTab.length(), l=files[0].length();i<l;i++) { insertChar(files[0][i]); }
                insertChar(' ');
              }

              // If no file has the entered prefix
              else if(files.size() == 0) mybeep();

              // If many files have the smae prefix
              else
              {
                // Search for longer prefix if possible
                for(int i=foreTab.length(), l=files[0].length();i<l;i++)
                {
                  for(int j=0, s=files.size();j<s;j++)
                  {
                    if(files[0][i] != files[j][i]) { add = false; break; }
                  }
                  if(add) { prefix += files[0][i]; }
                }

                // If the matching files do not have longer prefix, print their names
                if(prefix == "")
                {
                  for(int i=0, s=files.size();i<s;i++)
                  {
                    if(i%5 == 0) { cout << endl; }
                    cout << setw(16) << left << files[i];
                  }
                  reprintCmd();
                }

                // If there's a longer prefix, print it out
                else { for(int i=0, l=prefix.length();i<l;i++) insertChar(prefix[i]); } 
              }
            }
          }
        }
      }
    }

    // If the cursor is "on" the first word => use foreTab for matching
    else
    {
      for(map<string, CmdExec*>::iterator it=_cmdMap.begin(); it!=_cmdMap.end(); ++it)
      {
        command_1 = it->first;
        command_2 = it->second->getOptCmd();
        command_1 += command_2;

        if(foreTab.length() <= command_1.length())
        {
          int len;
          if(foreTab.length() > it->first.length()) { len = it->first.length(); }
          else { len = foreTab.length(); }

          // Find the matching commands and store them(upper-case) in matchcmd
          match = myStrNCmp(command_1, foreTab, len);
          if(match == 0) { matchCmd.push_back(it->first); }
        }
      }
    }
    
    // If multiple commands are matched, print the entire command
    if(matchCmd.size() > 1)
    {
      cout << endl;
      for(int i=0, cnt=1, s=matchCmd.size();i<s;i++, cnt++)
      {
        command_1 = matchCmd[i];
        command_2 = _cmdMap[matchCmd[i]]->getOptCmd();
        command_1 += command_2;
        cout << setw(12) << left << command_1;
        if(cnt%5==0) cout << endl;
      }
      reprintCmd();
    }

    // Only one command matched, print it out
    else if(matchCmd.size() == 1)
    {
      command_1 = matchCmd[0];
      command_2 = _cmdMap[matchCmd[0]]->getOptCmd();
      command_1 += command_2;

      for(int i=foreTab.length(), s=command_1.length();i<s;i++)
      {
        insertChar(command_1[i]);
      }
      insertChar(' ');
    }

    // If no command matched
    else { if(beep) mybeep(); }
  }
}
// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
  CmdExec* e = 0;
  // TODO...done
  int match, len;
  string command_1, command_2;

  for(map<string, CmdExec*>::iterator it=_cmdMap.begin(); it!=_cmdMap.end(); ++it)
  {
    // Store the full command in command_1
    command_1 = it->first;
    command_2 = it->second->getOptCmd();
    command_1 += command_2;

    // Fully matched or only the upper-case part neither will do
    /*match = myStrNCmp(it->first, cmd, it->first.length());
    if(match != 0)
    {
      if(cmd.length()<command_1.length()) { len = cmd.length(); }
      else { len = it->first.length(); }
      match = myStrNCmp(command_1, cmd, len);
    }*/
    if(cmd.length()>=it->first.length())
    {
      if(cmd.length()<command_1.length()) { len = cmd.length(); }
      else { len = it->first.length(); }
      match = myStrNCmp(command_1, cmd, len);
    
      if(match == 0) { e = it->second; break; }
    }
  }
  if(match != 0) { return 0; }

  return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// return false if option contains an token
bool
CmdExec::lexNoOption(const string& option) const
{
   string err;
   myStrGetTok(option, err);
   if (err.size()) {
      errorOption(CMD_OPT_EXTRA, err);
      return false;
   }
   return true;
}

// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
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

