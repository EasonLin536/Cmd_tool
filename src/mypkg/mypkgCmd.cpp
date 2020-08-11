// TODO: package cmd name
#include <iostream>
#include <iomanip>
#include <cassert>
#include "util.h"
// TODO: package name
#include "mypkgCmd.h"
#include "mypkg.h"


bool
initDbCmd()
{
  // TODO: add commands
  if (!(
        cmdMgr->regCmd("MYPKGCmd", 3, new MYPKGCmd)
     )) {
     cerr << "Registering \"init\" commands fails... exiting" << endl;
     return false;
  }

   return true;
}

// TODO: define methods for commands
// inspect src/cmd/cmdCommon.h & cmdCommon.cpp for some examples
//----------------------------------------------------------------------
//    MYPKGCmd <(string)>
//----------------------------------------------------------------------
CmdExecStatus
MYPKGCmd::exec(const string& option)
{
    /*
    return CmdExecStatus {
        CMD_EXEC_DONE ,
        CMD_EXEC_ERROR,
        CMD_EXEC_QUIT ,
        CMD_EXEC_NOP
    }
    */

    /*
    if CmdOptionError {
       CMD_OPT_MISSING   ,
       CMD_OPT_EXTRA     ,
       CMD_OPT_ILLEGAL   ,
       CMD_OPT_FOPEN_FAIL
    }
    use CmdExec::errorOption(CmdOptionError, err option string)
    */

   return CMD_EXEC_DONE;
}

void
MYPKGCmd::usage(ostream& os) const
{
    // os << "Usage: MYPKGCmd <(string)>" << endl;
}

void
MYPKGCmd::help() const
{
    // cout << setw(15) << left << "MYPKGCmd: "
    //     << "define new commands" << endl;
}