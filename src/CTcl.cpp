#include <CTcl.h>
#include <CStrParse.h>
#include <CStrUtil.h>
#include <CPrintF.h>
#include <CGlob.h>
#include <CRegExp.h>
#include <CCommand.h>
#include <CHistory.h>
#include <COSProcess.h>
#include <COSTimer.h>
#include <COSTime.h>
#include <COSUser.h>
#include <COSFile.h>
#include <CEval.h>
#include <CFile.h>
#include <CDir.h>
#include <CFileMatch.h>
#include <CTimer.h>
#include <CEnv.h>
#include <cmath>

class CTclParse : public CStrParse {
 public:
  CTclParse(CTcl *tcl, const std::string &filename);
 ~CTclParse();

  bool eof() const;

 private:
  bool fillBuffer();

 private:
  CTcl  *tcl_;
  CFile *file_;
};

//------

class CTclTimer : public CTimer {
 public:
  CTclTimer(CTcl *tcl, ulong ms, const std::string &script) :
   CTimer(ms), tcl_(tcl), script_(script), triggered_(false) {
    static uint id;

    id_ = id++;

    tcl_->insertTimer(this);
  }

 ~CTclTimer() {
    tcl_->clearTimer(this);
  }

  uint getId() const { return id_; }

  std::string getName() const {
    return "after#" + CStrUtil::toString(id_);
  }

  const std::string &getScript() const { return script_; }

  void timeOut() {
    triggered_ = true;
  }

  bool isTriggered() const { return triggered_; }

  void exec() {
    (void) tcl_->parseString(script_);

    delete this;
  }

 private:
  CTcl         *tcl_ { 0 };
  std::string  script_;
  uint         id_ { 0 };
  bool         triggered_ { false };
};

//------

CTcl::
CTcl(int argc, char **argv) :
 parse_(NULL), scope_(NULL), gscope_(NULL), separator_(';'), debug_(false)
{
  gscope_ = new CTclScope(this);

  pushScope(gscope_);

  addVariable("argc" , createValue(argc - 1));
  addVariable("argv0", createValue(argv[0]));

  std::vector<std::string> strs;

  for (int i = 1; i < argc; ++i)
    strs.push_back(argv[i]);

  addVariable("argv", createValue(strs));

  addVariable("env", CTclVariableRef(new CTclEnvVariable));

  history_ = new CHistory;

  //------

  addCommand(new CTclCommentCommand   (this));
  addCommand(new CTclAfterCommand     (this));
  addCommand(new CTclAppendCommand    (this));
//addCommand(new CTclApplyCommand     (this));
  addCommand(new CTclArrayCommand     (this));
//addCommand(new CTclBGErrorCommand   (this));
//addCommand(new CTclBinaryCommand    (this));
  addCommand(new CTclBreakCommand     (this));
  addCommand(new CTclCatchCommand     (this));
  addCommand(new CTclCDCommand        (this));
//addCommand(new CTclChanCommand      (this));
  addCommand(new CTclClockCommand     (this));
  addCommand(new CTclCloseCommand     (this));
  addCommand(new CTclContinueCommand  (this));
//addCommand(new CTclDictCommand      (this));
//addCommand(new CTclEncodingCommand  (this));
//addCommand(new CTclEchoCommand      (this));
  addCommand(new CTclEofCommand       (this));
//addCommand(new CTclErrorCommand     (this));
  addCommand(new CTclEvalCommand      (this));
  addCommand(new CTclExecCommand      (this));
  addCommand(new CTclExitCommand      (this));
  addCommand(new CTclExprCommand      (this));
//addCommand(new CTclFBlockedCommand  (this));
//addCommand(new CTclFConfigureCommand(this));
//addCommand(new CTclFCopyCommand     (this));
  addCommand(new CTclFileCommand      (this));
//addCommand(new CTclFileEventCommand (this));
  addCommand(new CTclFlushCommand     (this));
  addCommand(new CTclForCommand       (this));
  addCommand(new CTclForeachCommand   (this));
  addCommand(new CTclFormatCommand    (this));
  addCommand(new CTclGetsCommand      (this));
  addCommand(new CTclGlobCommand      (this));
  addCommand(new CTclGlobalCommand    (this));
  addCommand(new CTclHistoryCommand   (this));
  addCommand(new CTclIfCommand        (this));
  addCommand(new CTclIncrCommand      (this));
  addCommand(new CTclInfoCommand      (this));
//addCommand(new CTclInterpCommand    (this));
  addCommand(new CTclJoinCommand      (this));
  addCommand(new CTclLAppendCommand   (this));
//addCommand(new CTclLAssignCommand   (this));
  addCommand(new CTclLIndexCommand    (this));
  addCommand(new CTclLInsertCommand   (this));
  addCommand(new CTclListCommand      (this));
  addCommand(new CTclLLengthCommand   (this));
//addCommand(new CTclLoadCommand      (this));
  addCommand(new CTclLRangeCommand    (this));
  addCommand(new CTclLRepeatCommand   (this));
  addCommand(new CTclLReplaceCommand  (this));
//addCommand(new CTclLReverseCommand  (this));
  addCommand(new CTclLSearchCommand   (this));
  addCommand(new CTclLSetCommand      (this));
  addCommand(new CTclLSortCommand     (this));
  addCommand(new CTclNamespaceCommand (this));
  addCommand(new CTclOpenCommand      (this));
//addCommand(new CTclPackageCommand   (this));
//addCommand(new CTclPArrayCommand    (this));
  addCommand(new CTclPidCommand       (this));
  addCommand(new CTclProcCommand      (this));
  addCommand(new CTclPutsCommand      (this));
  addCommand(new CTclPwdCommand       (this));
  addCommand(new CTclReadCommand      (this));
//addCommand(new CTclRegexpCommand    (this));
//addCommand(new CTclRenameCommand    (this));
  addCommand(new CTclReturnCommand    (this));
//addCommand(new CTclScanCommand      (this));
//addCommand(new CTclSeekCommand      (this));
  addCommand(new CTclSetCommand       (this));
//addCommand(new CTclSocketCommand    (this));
  addCommand(new CTclSourceCommand    (this));
//addCommand(new CTclSplitCommand     (this));
  addCommand(new CTclStringCommand    (this));
//addCommand(new CTclSubstCommand     (this));
  addCommand(new CTclSwitchCommand    (this));
//addCommand(new CTclTellCommand      (this));
//addCommand(new CTclTimeCommand      (this));
//addCommand(new CTclTraceCommand     (this));
  addCommand(new CTclUnsetCommand     (this));
  addCommand(new CTclUpdateCommand    (this));
//addCommand(new CTclUplevelCommand   (this));
//addCommand(new CTclUpvarCommand     (this));
  addCommand(new CTclVariableCommand  (this));
//addCommand(new CTclVWaitCommand     (this));
  addCommand(new CTclWhileCommand     (this));
}

CTcl::
~CTcl()
{
  CommandList::iterator p1, p2;

  for (p1 = cmds_.begin(), p2 = cmds_.end(); p1 != p2; ++p1)
    delete (*p1).second;

  delete scope_;

  popScope();

  delete history_;
}

bool
CTcl::
isCompleteLine(const std::string &line)
{
  startStringParse(line);

  bool rc = isCompleteLine1('\0');

  endParse();

  return rc;
}

bool
CTcl::
isCompleteLine1(char endChar)
{
  while (! parse_->eof()) {
    if      (parse_->isChar('[')) {
      parse_->skipChar();

      if (! isCompleteLine1(']'))
        return false;

      if (! parse_->isChar(']'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('{')) {
      parse_->skipChar();

      if (! isCompleteLine1('}'))
        return false;

      if (! parse_->isChar('}'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar(endChar)) {
      return true;
    }
    else if (parse_->isChar('\"')) {
      parse_->skipChar();

      if (! isCompleteLine1('\"'))
        return false;

      if (! parse_->isChar('\"'))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('\'')) {
      parse_->skipChar();

      if (! isCompleteLine1('\''))
        return false;

      if (! parse_->isChar('\''))
        return false;

      parse_->skipChar();
    }
    else if (parse_->isChar('\\')) {
      parse_->skipChar();
      parse_->skipChar();
    }
    else
      parse_->skipChar();
  }

  return true;
}

bool
CTcl::
parseFile(const std::string &filename)
{
  if (! CFile::isRegular(filename)) {
    std::cerr << "Invalid file " << filename << std::endl;
    return false;
  }

  startFileParse(filename);

  bool rc = true;

  while (! parse_->eof()) {
    std::vector<CTclValueRef> args;

    try {
      if (! readArgList(args)) {
        rc = false;
        break;
      }

      CTclValueRef value = evalArgs(args);

      if (getDebug() && value.isValid()) {
        value->print(std::cerr);

        std::cerr << std::endl;
      }
    }
    catch (CTclError err) {
      std::cerr << err.getMsg() << std::endl;
      break;
    }
  }

  endParse();

  return rc;
}

CTclValueRef
CTcl::
parseLine(const std::string &str)
{
  CTclValueRef value;

  try {
    value = parseString(str);

    addHistory(str);
  }
  catch (CTclError err) {
    std::cerr << err.getMsg() << std::endl;
  }

  return value;
}

CTclValueRef
CTcl::
parseString(const std::string &str)
{
  CTclValueRef value;

  startStringParse(str);

  bool rc = true;

  while (! parse_->eof()) {
    std::vector<CTclValueRef> args;

    if (! readArgList(args)) {
      rc = false;
      break;
    }

    value = evalArgs(args);

    if (getDebug() && value.isValid()) {
      value->print(std::cerr);

      std::cerr << std::endl;
    }

    if (getBreakFlag() || getContinueFlag() || getReturnFlag())
      break;
  }

  endParse();

  if (! rc)
    return CTclValueRef();

  return value;
}

bool
CTcl::
readArgList(std::vector<CTclValueRef> &args)
{
  while (! parse_->eof()) {
    while (parse_->isChar(' ') || parse_->isChar('\t'))
      parse_->skipChar();

    if (parse_->eof())
      return true;

    if      (parse_->isChar(';') || parse_->isChar('\n')) {
      parse_->skipChar();

      return true;
    }
    else if (parse_->isChar('[')) {
      std::vector<CTclValueRef> args1;

      if (! readExecString(args1))
        return false;

      CTclValueRef value = evalArgs(args1);

      if (! value.isValid()) {
        std::cerr << "Invalid value" << std::endl;
        return false;
      }

      args.push_back(value);
    }
    else if (parse_->isChar(']')) {
      return true;
    }
    else if (parse_->isChar('{')) {
      std::string str;

      if (! readLiteralString(str))
        return false;

      args.push_back(createValue(str));
    }
    else if (parse_->isChar('\"')) {
      std::string str;

      if (! readDoubleQuotedString(str))
        return false;

      args.push_back(createValue(str));
    }
    else if (parse_->isChar('\'')) {
      std::string str;

      if (! readSingleQuotedString(str))
        return false;

      args.push_back(createValue(str));
    }
    else if (parse_->isChar('$')) {
      parse_->skipChar();

      std::string varName, indexName;
      bool   is_array;

      if (! readVariableName(varName, indexName, is_array))
        return false;

      CTclValueRef value;

      if (is_array)
        value = getArrayVariableValue(varName, indexName);
      else
        value = getVariableValue(varName);

      if (! value.isValid()) {
        throwError("can't read \"" + varName + "\": no such variable");
        return false;
      }

      if (parse_->isSpace())
        args.push_back(value);
      else {
        std::string str1 = value->toString();

        std::string str2;

        if (! readWord(str2, ';'))
          return false;

        args.push_back(createValue(str1 + str2));
      }
    }
    else {
      std::string str;

      if (! readWord(str, ';'))
        return false;

      args.push_back(createValue(str));
    }
  }

  return true;
}

bool
CTcl::
readExecString(std::vector<CTclValueRef> &args)
{
  assert(parse_->isChar('['));

  parse_->skipChar();

  parse_->skipSpace();

  while (! parse_->isChar(']')) {
    if      (parse_->isChar('{')) {
      std::string str1;

      if (! readLiteralString(str1))
        return false;

      args.push_back(createValue(str1));
    }
    else if (parse_->isChar('\"')) {
      std::string str1;

      if (! readDoubleQuotedString(str1))
        return false;

      args.push_back(createValue(str1));
    }
    else if (parse_->isChar('\'')) {
      std::string str1;

      if (! readSingleQuotedString(str1))
        return false;

      args.push_back(createValue(str1));
    }
    else {
      std::string str;

      if (! readWord(str, ']'))
        return false;

      args.push_back(createValue(str));
    }

    parse_->skipSpace();
  }

  parse_->skipChar();

  return true;
}

CTclList *
CTcl::
stringToList(const std::string &str)
{
  CTclList *list = new CTclList;

  startStringParse(str);

  while (! parse_->eof()) {
    while (parse_->isSpace())
      parse_->skipChar();

    if (parse_->eof())
      break;

    if      (parse_->isChar('{')) {
      std::string str1;

      if (! readLiteralString(str1))
        break;

      list->addValue(createValue(str1));
    }
    else if (parse_->isChar('\"')) {
      std::string str1;

      if (! readDoubleQuotedString(str1))
        break;

      list->addValue(createValue(str1));
    }
    else {
      std::string str1;

      while (! parse_->eof()) {
        if (parse_->isSpace())
          break;

        if (parse_->isChar('\\')) {
          parse_->skipChar();

          char c;

          if (! parse_->readChar(&c)) {
            std::cerr << "Invalid char after \\" << std::endl;
            break;
          }

          str1 += c;
        }
        else {
          char c;

          if (! parse_->readChar(&c)) {
            std::cerr << "Invalid char" << std::endl;
            break;
          }

          str1 += c;
        }
      }

      list->addValue(createValue(str1));
    }
  }

  endParse();

  return list;
}

bool
CTcl::
readLiteralString(std::string &str)
{
  assert(parse_->isChar('{'));

  SetSeparator sepSep(this, '\n');

  parse_->skipChar();

  while (! parse_->eof() && ! parse_->isChar('}')) {
    if (parse_->isChar('{')) {
      std::string str1;

      if (! readLiteralString(str1))
        return false;

      str += "{" + str1 + "}";
    }
    else {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Unterminated string" << std::endl;
        return false;
      }

      str += c;
    }
  }

  if (! parse_->isChar('}')) {
    std::cerr << "Unterminated string" << std::endl;
    return false;
  }

  parse_->skipChar();

  return true;
}

bool
CTcl::
readDoubleQuotedString(std::string &str)
{
  assert(parse_->isChar('\"'));

  parse_->skipChar();

  while (! parse_->isChar('\"')) {
    if      (parse_->isChar('[')) {
      std::vector<CTclValueRef> args1;

      if (! readExecString(args1))
        return false;

      CTclValueRef value = evalArgs(args1);

      if (! value.isValid()) {
        std::cerr << "Invalid value" << std::endl;
        return false;
      }

      str += value->toString();
    }
    else if (parse_->isChar('$')) {
      parse_->skipChar();

      std::string varName, indexName;
      bool        is_array;

      if (! readVariableName(varName, indexName, is_array))
        return false;

      CTclValueRef value;

      if (is_array)
        value = getArrayVariableValue(varName, indexName);
      else
        value = getVariableValue(varName);

      if (! value.isValid()) {
        throwError("can't read \"" + varName + "\": no such variable");
        return false;
      }

      str += value->toString();
    }
    else if (parse_->isChar('\\')) {
      parse_->skipChar();

      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char after \\" << std::endl;
        return false;
      }

      switch (c) {
        case 'a': str += '\a'; break;
        case 'b': str += '\b'; break;
        case 'f': str += '\f'; break;
        case 'n': str += '\n'; break;
        case 'r': str += '\r'; break;
        case 't': str += '\t'; break;
        case 'v': str += '\v'; break;
        default : str += c   ; break;

        // octal
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': {
          char value = c - '0';

          int num = 1;

          while (! parse_->eof()) {
            if (! parse_->readChar(&c)) {
              std::cerr << "Invalid octal" << std::endl;
              return false;
            }

            if (! CStrUtil::isodigit(c)) {
              parse_->unreadChar();
              break;
            }

            value = (value << 3) | (c - '0');

            ++num;

            if (num == 3) break;
          }

          str += value;

          break;
        }

        // hex
        case 'x': {
          char value = 0;

          while (! parse_->eof()) {
            if (! parse_->readChar(&c)) {
              std::cerr << "Invalid hex" << std::endl;
              return false;
            }

            if (! isxdigit(c)) {
              parse_->unreadChar();
              break;
            }

            char value1 = 0;

            if (isdigit(c))
              value1 = c - '0';
            else if (islower(c))
              value1 = c - 'a';
            else
              value1 = c - 'A';

            value = (value << 4) | value1;
          }

          str += value;

          break;
        }
      }
    }
    else {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char" << std::endl;
        return false;
      }

      str += c;
    }
  }

  parse_->skipChar();

  return true;
}

bool
CTcl::
readSingleQuotedString(std::string &str)
{
  assert(parse_->isChar('\''));

  parse_->skipChar();

  while (! parse_->isChar('\'')) {
    char c;

    if (! parse_->readChar(&c)) {
      std::cerr << "Unterminated string" << std::endl;
      return false;
    }

    str += c;
  }

  parse_->skipChar();

  return true;
}

bool
CTcl::
readVariableName(std::string &varName, std::string &indexName, bool &is_array)
{
  // ${name} - name can have any characters
  if (parse_->isChar('{')) {
    if (! readLiteralString(varName))
      return false;
  }
  // $name - sequence of one or more characters that are a letter, digit,
  // underscore, or namespace separators (two or more colons)
  else {
    while (! parse_->eof()) {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Missing variable char" << std::endl;
        return false;
      }

      if (! isalnum(c) && c != '_' && c != ':') {
        parse_->unreadChar();
        break;
      }

      varName += c;
    }
  }

  is_array = false;

  // $name(index) - name must contain only letters, digits, underscores, and namespace
  // separators, and may be an empty string. Command substitutions, variable substitutions,
  // and backslash substitutions are performed on the characters of index.
  if (parse_->isChar('(')) {
    parse_->skipChar();

    std::string str2;

    int depth = 1;

    while (! parse_->eof()) {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Missing variable char" << std::endl;
        return false;
      }

      if      (c == '(')
        ++depth;
      else if (c == ')') {
        --depth;

        if (depth == 0) break;
      }

      str2 += c;
    }

    if (depth != 0) {
      std::cerr << "Invalid () nesting" << std::endl;
      return false;
    }

    indexName = expandExpr(str2);

    is_array = true;
  }

  return true;
}

bool
CTcl::
readWord(std::string &str, char endChar)
{
  assert(! parse_->isSpace());

  while (! parse_->eof()) {
    if (parse_->isSpace() || parse_->isChar(endChar))
      break;

    if      (parse_->isChar('[')) {
      std::vector<CTclValueRef> args1;

      if (! readExecString(args1))
        return false;

      CTclValueRef value = evalArgs(args1);

      if (! value.isValid()) {
        std::cerr << "Invalid value" << std::endl;
        return false;
      }

      str += value->toString();
    }
    else if (parse_->isChar('$')) {
      parse_->skipChar();

      std::string varName, indexName;
      bool        is_array;

      if (! readVariableName(varName, indexName, is_array))
        return false;

      CTclValueRef value;

      if (is_array)
        value = getArrayVariableValue(varName, indexName);
      else
        value = getVariableValue(varName);

      if (! value.isValid()) {
        throwError("can't read \"" + varName + "\": no such variable");
        return false;
      }

      str += value->toString();
    }
    else if (parse_->isChar('\\')) {
      parse_->skipChar();

      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char after \\" << std::endl;
        return false;
      }

      str += c;
    }
    else {
      char c;

      if (! parse_->readChar(&c)) {
        std::cerr << "Invalid char" << std::endl;
        return false;
      }

      str += c;
    }
  }

  return true;
}

std::string
CTcl::
expandExpr(const std::string &str)
{
  std::string str1;

  startStringParse(str);

  while (! parse_->eof()) {
    char c;

    if (! parse_->readChar(&c))
      break;

    if (c == '$') {
      std::string varName, indexName;
      bool        is_array;

      if (! readVariableName(varName, indexName, is_array))
        break;

      CTclValueRef value;

      if (is_array)
        value = getArrayVariableValue(varName, indexName);
      else
        value = getVariableValue(varName);

      if (! value.isValid()) {
        throwError("can't read \"" + varName + "\": no such variable");
        return "";
      }

      str1 += value->toString();
    }
    else
      str1 += c;
  }

  endParse();

  return str1;
}

void
CTcl::
addHistory(const std::string &str)
{
  history_->addCommand(str);
}

CTclScope *
CTcl::
getNamedScope(const std::string &name, bool create_it)
{
  return scope_->getNamedScope(name, create_it);
}

void
CTcl::
pushScope(CTclScope *scope)
{
  scopeStack_.push_back(scope_);

  scope_ = scope;
}

void
CTcl::
popScope()
{
  assert(! scopeStack_.empty());

  scope_ = scopeStack_.back();

  scopeStack_.pop_back();
}

void
CTcl::
unwindScope()
{
  while (! scopeStack_.empty())
    popScope();

  pushScope(gscope_);
}

void
CTcl::
startCommand(CTclCommand *cmd)
{
  cmdStack_.push_back(cmd);

  if (getDebug()) std::cerr << "Start: " << cmdStack_.back()->getName() << std::endl;
}

void
CTcl::
endCommand()
{
  assert(! cmdStack_.empty());

  if (getDebug()) std::cerr << "End: " << cmdStack_.back()->getName() << std::endl;

  cmdStack_.pop_back();
}

CTclCommand *
CTcl::
getCommand() const
{
  if (cmdStack_.empty()) return NULL;

  return cmdStack_.back();
}

void
CTcl::
startProc(CTclProc *proc)
{
  procStack_.push_back(proc);

  if (getDebug()) std::cerr << "Start: " << procStack_.back()->getName() << std::endl;
}

void
CTcl::
endProc()
{
  assert(! procStack_.empty());

  if (getDebug()) std::cerr << "End: " << procStack_.back()->getName() << std::endl;

  procStack_.pop_back();
}

CTclProc *
CTcl::
getProc() const
{
  if (procStack_.empty()) return NULL;

  return procStack_.back();
}

void
CTcl::
addCommand(CTclCommand *command)
{
  cmds_[command->getName()] = command;
}

CTclCommand *
CTcl::
getCommand(const std::string &name)
{
  CommandList::const_iterator p = cmds_.find(name);

  if (p == cmds_.end())
    return NULL;

  return (*p).second;
}

void
CTcl::
getCommandNames(std::vector<std::string> &names) const
{
  CommandList::const_iterator p1, p2;

  for (p1 = cmds_.begin(), p2 = cmds_.end(); p1 != p2; ++p1)
    names.push_back((*p1).first);
}

CTclVariableRef
CTcl::
addVariable(const std::string &varName, CTclValueRef value)
{
  return getScope()->addVariable(varName, value);
}

CTclVariableRef
CTcl::
addVariable(const std::string &varName, CTclVariableRef var)
{
  return getScope()->addVariable(varName, var);
}

CTclVariableRef
CTcl::
getVariable(const std::string &varName)
{
  bool global = false;

  std::vector<std::string> names;

  uint len = varName.size();
  uint pos = 0;

  while (pos < len) {
    if (varName[pos] == ':') {
      ++pos;

      while (pos < len && varName[pos] == ':')
        ++pos;

      if (names.empty())
        global = true;
    }

    if (pos < len) {
      std::string name;

      while (pos < len && varName[pos] != ':')
        name += varName[pos++];

      names.push_back(name);
    }
  }

  uint num_names = names.size();

  if (num_names > 1) {
    CTclScope *scope = getGlobalScope();

    for (uint i = 0; i < num_names - 1; ++i) {
      scope = scope->getNamedScope(names[i]);

      if (scope == NULL)
        return CTclVariableRef();
    }

    CTclVariableRef var = scope->getVariable(names[num_names - 1]);

    if (var.isValid())
      return var;
  }
  else {
    if (global) {
      CTclScope *scope = getGlobalScope();

      CTclVariableRef var = scope->getVariable(names[0]);

      if (var.isValid())
        return var;
    }
    else {
      CTclScope *scope = getScope();

      while (scope) {
        CTclVariableRef var = scope->getVariable(varName);

        if (var.isValid())
          return var;

        CTclScope *parentScope = scope->parentScope();

        if (parentScope == NULL)
          break;

        scope = parentScope;
      }
    }
  }

  return CTclVariableRef();
}

CTclValueRef
CTcl::
getVariableValue(const std::string &varName)
{
  CTclVariableRef var = getVariable(varName);

  if (var.isValid())
    return var->getValue();
  else
    return CTclValueRef();
}

void
CTcl::
setVariableValue(const std::string &varName, CTclValueRef value)
{
  CTclScope *scope = getScope();

  while (scope) {
    CTclVariableRef var = scope->getVariable(varName);

    if (var.isValid()) {
      var->setValue(value);
      return;
    }

    CTclScope *parentScope = scope->parentScope();

    if (parentScope == NULL)
      break;

    scope = parentScope;
  }

  if (scope)
    scope->setVariableValue(varName, value);
}

CTclValueRef
CTcl::
getArrayVariableValue(const std::string &varName, const std::string &indexStr)
{
  CTclVariableRef var = getVariable(varName);

  if (var.isValid())
    return var->getArrayValue(indexStr);
  else
    return CTclValueRef();
}

void
CTcl::
setArrayVariableValue(const std::string &varName, const std::string &indexStr, CTclValueRef value)
{
  CTclVariableRef var = getVariable(varName);

  if (! var.isValid()) {
    CTclArray *array = new CTclArray;

    array->setValue(indexStr, value);

    setVariableValue(varName, CTclValueRef(array));
  }
  else
    return var->setArrayValue(indexStr, value);
}

void
CTcl::
removeVariable(const std::string &varName)
{
  getScope()->removeVariable(varName);
}

CTclProc *
CTcl::
defineProc(const std::string &name, const std::vector<std::string> &args, CTclValueRef body)
{
  return getScope()->defineProc(name, args, body);
}

CTclProc *
CTcl::
getProc(const std::string &name)
{
  CTclScope *scope = getScope();

  while (scope) {
    CTclProc *proc = scope->getProc(name);

    if (proc) return proc;

    scope = scope->parentScope();
  }

  return NULL;
}

std::string
CTcl::
openFile(const std::string &fileName, const std::string &mode)
{
  FILE *fp = fopen(fileName.c_str(), mode.c_str());

  if (fp == NULL) {
    if      (errno == ENOENT)
      throwError("File does not exist");
    else if (errno == EACCES)
      throwError("No read permission for file");
    else
      throwError("Open Failed");

    return "";
  }

  int num = fileno(fp);

  std::string handle = "file" + CStrUtil::toString(num);

  fileMap_[handle] = fp;

  return handle;
}

void
CTcl::
closeFile(const std::string &handle)
{
  FileMap::iterator p = fileMap_.find(handle);

  if (p == fileMap_.end())
    return;

  fclose((*p).second);

  fileMap_.erase(p);
}

void
CTcl::
writeToFile(const std::string &handle, const std::string &str)
{
  FileMap::iterator p = fileMap_.find(handle);

  if (p == fileMap_.end())
    return;

  fwrite(str.c_str(), 1, str.size(), (*p).second);
}

FILE *
CTcl::
getFile(const std::string &handle)
{
  FileMap::iterator p = fileMap_.find(handle);

  if (p == fileMap_.end())
    return NULL;

  return (*p).second;
}

std::string
CTcl::
addTimer(int ms, const std::string &script)
{
  CTclTimer *timer = new CTclTimer(this, ms, script);

  return timer->getName();
}

bool
CTcl::
cancelTimer(const std::string &id)
{
  TimerMap::iterator p = timerMap_.find(id);

  if (p == timerMap_.end())
    return false;

  delete (*p).second;

  return true;
}

void
CTcl::
insertTimer(CTclTimer *timer)
{
  std::string name = timer->getName();

  timerMap_[name] = timer;
}

void
CTcl::
clearTimer(CTclTimer *timer)
{
  std::string name = timer->getName();

  TimerMap::iterator p = timerMap_.find(name);

  if (p != timerMap_.end())
    timerMap_.erase(p);
}

void
CTcl::
getTimerNames(std::vector<std::string> &names)
{
  TimerMap::const_iterator p1, p2;

  for (p1 = timerMap_.begin(), p2 = timerMap_.end(); p1 != p2; ++p1)
    names.push_back((*p1).first);
}

CTclTimer *
CTcl::
getTimer(const std::string &id)
{
  TimerMap::iterator p = timerMap_.find(id);

  if (p == timerMap_.end())
    return NULL;

  return (*p).second;
}

void
CTcl::
update(bool /*idletasks*/)
{
  std::vector<CTclTimer *> timers;

  TimerMap::const_iterator p1, p2;

  for (p1 = timerMap_.begin(), p2 = timerMap_.end(); p1 != p2; ++p1) {
    CTclTimer *timer = (*p1).second;

    if (timer->isTriggered())
      timers.push_back(timer);
  }

  uint num_timers = timers.size();

  for (uint i = 0; i < num_timers; ++i)
    timers[i]->exec();
}

CTclValueRef
CTcl::
createValue(int value) const
{
  return CTclValueRef(new CTclString(CStrUtil::toString(value)));
}

CTclValueRef
CTcl::
createValue(uint value) const
{
  return CTclValueRef(new CTclString(CStrUtil::toString(value)));
}

CTclValueRef
CTcl::
createValue(double value) const
{
  return CTclValueRef(new CTclString(CStrUtil::toString(value)));
}

CTclValueRef
CTcl::
createValue(const std::string &str) const
{
  return CTclValueRef(new CTclString(str));
}

CTclValueRef
CTcl::
createValue(const std::vector<std::string> &strs) const
{
  CTclList *list = new CTclList;

  uint numStrs = strs.size();

  for (uint i = 0; i < numStrs; ++i)
    list->addValue(createValue(strs[i]));

  return CTclValueRef(list);
}

CTclValueRef
CTcl::
createValue(const std::vector<CTclValueRef> &values) const
{
  CTclList *list = new CTclList;

  uint numValues = values.size();

  for (uint i = 0; i < numValues; ++i)
    list->addValue(values[i]);

  return CTclValueRef(list);
}

CTclValueRef
CTcl::
evalString(const std::string &str)
{
  std::string str1 = expandExpr(str);

  CEval eval;

  double result;

  if (! eval.eval(str1, &result)) {
    throwError("error in expression \"" + str1 + "\"");
    return CTclValueRef();
  }

  int ires = int(fabs(result) + 1E-6);

  if      (fabs(result) - ires > 1E-6)
    return CTclValueRef(new CTclString(CStrUtil::toString(result)));
  else if (result < 0)
    return CTclValueRef(new CTclString(CStrUtil::toString(-ires)));
  else
    return CTclValueRef(new CTclString(CStrUtil::toString(ires)));
}

CTclValueRef
CTcl::
evalArgs(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) return CTclValueRef();

  std::string name = args[0]->toString();

  CTclCommand *cmd = getCommand(name);

  if (cmd) {
    std::vector<CTclValueRef> args1;

    if (getDebug())
      std::cerr << "Exec:" << name;

    for (uint i = 1; i < numArgs; ++i) {
      if (getDebug())
        std::cerr << " " << args[i];

      args1.push_back(args[i]);
    }

    if (getDebug())
      std::cerr << std::endl;

    startCommand(cmd);

    CTclValueRef ret = cmd->exec(args1);

    endCommand();

    return ret;
  }
  else {
    CTclProc *proc = getProc(name);

    if (proc) {
      std::vector<CTclValueRef> args1;

      for (uint i = 1; i < numArgs; ++i)
        args1.push_back(args[i]);

      startProc(proc);

      CTclValueRef ret = proc->exec(args1);

      endProc();

      return ret;
    }

    //-------

    std::vector<CCommand *> cmds;

    bool ignore_arg = false;

    std::string path = lookupPathCommand(name);

    if (path == "") {
      throwError("invalid command name \"" + name + "\"");
      return CTclValueRef();
    }

    CCommand *command = new CCommand(name, path);

    cmds.push_back(command);

//CCommandMgrInst->setDebug(true);

    for (uint i = 1; i < numArgs; ++i) {
      std::string arg = args[i]->toString();

      if (ignore_arg) {
        command->addArg(arg);

        continue;
      }

      if      (arg == "--") {
        ignore_arg = true;
      }
      else if (arg == "|" || arg == "|&") {
        if (i >= numArgs - 1) {
          throwError("illegal use of | or |& in command");
          return CTclValueRef();
        }

        ignore_arg = false;

        ++i;

        command->addPipeDest(1);

        if (arg == "|&")
          command->addPipeDest(2);

        name = args[i]->toString();
        path = lookupPathCommand(name);

        if (path == "") {
          throwError("couldn't execute \"" + name + "\" no such file or directory");
          return CTclValueRef();
        }

        command = new CCommand(name, path);

        cmds.push_back(command);

        command->addPipeSrc();
      }
      else if (arg == "<") {
        if (i >= numArgs - 1) {
          throwError("can't specify \"" + arg + "\" as last word in command");
          return CTclValueRef();
        }

        ++i;

        const std::string &src = args[i]->toString();

        command->addFileSrc(src);
      }
      else if (arg == "<@") {
        if (i >= numArgs - 1) {
          throwError("can't specify \"" + arg + "\" as last word in command");
          return CTclValueRef();
        }

        ++i;

        const std::string &fileId = args[i]->toString();

        FILE *fp = NULL;

        if (fileId == "stdin")
          fp = stdin;
        else {
          fp = getFile(fileId);

          if (fp == NULL) {
            throwError("can not find channel named \"" + fileId + "\"");
            return CTclValueRef();
          }
        }

        command->addFileSrc(fp);
      }
      else if (arg == "<<") {
      }
      else if (arg == ">" || arg == ">&") {
        if (i >= numArgs - 1) {
          throwError("can't specify \"" + arg + "\" as last word in command");
          return CTclValueRef();
        }

        ++i;

        const std::string &dest = args[i]->toString();

        command->addFileDest(dest);

        if (arg == ">&")
          command->addFileDest(dest, 2);
      }
      else if (arg == "2>") {
      }
      else if (arg == ">>" || arg == ">>&") {
        if (i >= numArgs - 1) {
          throwError("can't specify \"" + arg + "\" as last word in command");
          return CTclValueRef();
        }

        ++i;

        const std::string &dest = args[i]->toString();

        command->addFileDest(dest);

        if (arg == ">>&")
          command->addFileDest(dest, 2);

        command->setFileDestAppend(true);
      }
      else if (arg == "2>>") {
      }
      else if (arg == ">@" || arg == ">&@") {
        if (i >= numArgs - 1) {
          throwError("can't specify \"" + arg + "\" as last word in command");
          return CTclValueRef();
        }

        ++i;

        const std::string &fileId = args[i]->toString();

        FILE *fp = NULL;

        if      (fileId == "stdout")
          fp = stdout;
        else if (fileId == "stderr")
          fp = stderr;
        else {
          fp = getFile(fileId);

          if (fp == NULL) {
            throwError("can not find channel named \"" + fileId + "\"");
            return CTclValueRef();
          }
        }

        command->addFileDest(fp);

        if (arg == ">&%")
          command->addFileDest(fp, 2);
      }
      else if (arg == "2>@") {
      }
      else if (arg == "2>@1") {
      }
      else
        command->addArg(arg);
    }

    uint num_cmds = cmds.size();

    for (uint i = 0; i < num_cmds; ++i) {
      CCommand *cmd = cmds[i];

      if (i == 0)
        cmd->setProcessGroupLeader();
      else
        cmd->setProcessGroup(cmds[0]);
    }

    for (uint i = 0; i < num_cmds; ++i) {
      CCommand *cmd = cmds[i];

      cmd->start();
    }

    for (uint i = 0; i < num_cmds; ++i) {
      CCommand *cmd = cmds[i];

      cmd->wait();
    }

    int rc = cmds[0]->getReturnCode();

    for (uint i = 0; i < num_cmds; ++i)
      delete cmds[i];

    if (rc != 0)
      throwError("child process exited abnormally (" + CStrUtil::toString(rc) + ")");

    return CTclValueRef();
  }
}

std::string
CTcl::
lookupPathCommand(const std::string &name) const
{
  std::string value;

  if (! CEnvInst.get("PATH", value))
    return "";

  std::vector<std::string> words;

  CStrUtil::addFields(value, words, ":");

  uint num_words = words.size();

  for (uint i = 0; i < num_words; ++i) {
    std::string path = words[i] + "/" + name;

    if (! CFile::isRegular(path) || ! CFile::isExecutable(path))
      continue;

    return path;
  }

  return "";
}

void
CTcl::
startFileParse(const std::string &fileName)
{
  parseStack_.push_back(parse_);

  parse_ = new CTclParse(this, fileName);
}

void
CTcl::
startStringParse(const std::string &str)
{
  parseStack_.push_back(parse_);

  parse_ = new CStrParse(str);
}

void
CTcl::
endParse()
{
  delete parse_;

  parse_ = parseStack_.back();

  parseStack_.pop_back();
}

void
CTcl::
wrongNumArgs(const std::string &msg)
{
  throwError("wrong # args: should be \"" + msg + "\"");
}

void
CTcl::
badInteger(CTclValueRef value)
{
  throwError("expected integer but got \"" + value->toString() + "\"");
}

void
CTcl::
badReal(CTclValueRef value)
{
  throwError("expected number but got \"" + value->toString() + "\"");
}

void
CTcl::
throwError(const std::string &msg)
{
  unwindScope();

  throw CTclError(msg);
}

bool
CTcl::
needsBraces(const std::string &str)
{
  uint len = str.size();

  bool is_space = false;

  for (uint i = 0; i < len; ++i) {
    if (isspace(str[i])) {
      is_space = true;
      break;
    }
  }

  return (len == 0 || is_space);
}

//--------------

CTclParse::
CTclParse(CTcl *tcl, const std::string &filename) :
 tcl_(tcl)
{
  file_ = new CFile(filename);
}

CTclParse::
~CTclParse()
{
  delete file_;
}

bool
CTclParse::
fillBuffer()
{
  std::string line;

  if (! file_->readLine(line))
    return false;

  uint len = line.size();

  while (len > 0 && line[len - 1] == '\\') {
    line = line.substr(0, len - 1);

    std::string line1;

    if (file_->readLine(line1)) {
      uint len1 = line1.size();

      uint i = 0;

      while (i < len1 && isspace(line1[i]))
        ++i;

      line += " " + line1.substr(0);
    }

    len = line.size();
  }

  if (getPos() > 0)
    addString(tcl_->getSeparator() + line);
  else
    addString(line);

  return true;
}

bool
CTclParse::
eof() const
{
  if (! CStrParse::eof())
    return false;

  CTclParse *th = const_cast<CTclParse *>(this);

  th->fillBuffer();

  if (! CStrParse::eof())
    return false;

  return file_->eof();
}

//-----------

CTclScope::
CTclScope(CTcl *tcl, CTclScope *parent, const std::string &name) :
 tcl_(tcl), parent_(parent), name_(name)
{
}

CTclScope::
~CTclScope()
{
}

CTclVariableRef
CTclScope::
addVariable(const std::string &varName, CTclValueRef value)
{
  CTclVariable *var = new CTclVariable(value);

  return addVariable(varName, CTclVariableRef(var));
}

CTclVariableRef
CTclScope::
addVariable(const std::string &varName, CTclVariableRef var)
{
  assert(var.isValid());

  removeVariable(varName);

  vars_[varName] = var;

  return var;
}

CTclVariableRef
CTclScope::
getVariable(const std::string &varName)
{
  VariableList::const_iterator p = vars_.find(varName);

  if (p == vars_.end())
    return CTclVariableRef();

  return (*p).second;
}

CTclValueRef
CTclScope::
getVariableValue(const std::string &varName)
{
  CTclVariableRef var = getVariable(varName);

  if (var.isValid())
    return var->getValue();
  else
    return CTclValueRef();
}

CTclValueRef
CTclScope::
getArrayVariableValue(const std::string &varName, const std::string &indexStr)
{
  CTclVariableRef var = getVariable(varName);

  if (var.isValid())
    return var->getArrayValue(indexStr);
  else
    return CTclValueRef();
}

void
CTclScope::
setArrayVariableValue(const std::string &varName, const std::string &indexStr, CTclValueRef value)
{
  CTclVariableRef var = getVariable(varName);

  if (! var.isValid()) {
    CTclArray *array = new CTclArray;

    array->setValue(indexStr, value);

    setVariableValue(varName, CTclValueRef(array));
  }
  else
    return var->setArrayValue(indexStr, value);
}

void
CTclScope::
getVariableNames(std::vector<std::string> &names) const
{
  VariableList::const_iterator p1, p2;

  for (p1 = vars_.begin(), p2 = vars_.end(); p1 != p2; ++p1) {
    const std::string &name = (*p1).first;

    names.push_back(name);
  }
}

void
CTclScope::
setVariableValue(const std::string &varName, CTclValueRef value)
{
  VariableList::iterator p = vars_.find(varName);

  if (p == vars_.end()) {
    CTclVariable *var = new CTclVariable;

    p = vars_.insert(p, VariableList::value_type(varName, CTclVariableRef(var)));
  }

  CTclVariableRef var = (*p).second;

  var->setValue(value);
}

void
CTclScope::
removeVariable(const std::string &varName)
{
  VariableList::iterator p = vars_.find(varName);

  if (p != vars_.end())
    vars_.erase(p);
}

CTclProc *
CTclScope::
defineProc(const std::string &name, const std::vector<std::string> &args, CTclValueRef body)
{
  removeProc(name);

  CTclProc *proc = new CTclProc(tcl_, name, args, body);

  procs_[name] = proc;

  return proc;
}

CTclProc *
CTclScope::
getProc(const std::string &varName)
{
  ProcList::const_iterator p = procs_.find(varName);

  if (p == procs_.end())
    return NULL;

  return (*p).second;
}

void
CTclScope::
removeProc(const std::string &name)
{
  ProcList::iterator p = procs_.find(name);

  if (p != procs_.end()) {
    CTclProc *proc = (*p).second;

    procs_.erase(p);

    delete proc;
  }
}

void
CTclScope::
getProcNames(std::vector<std::string> &names)
{
  ProcList::const_iterator p1, p2;

  for (p1 = procs_.begin(), p2 = procs_.end(); p1 != p2; ++p1) {
    CTclProc *proc = (*p1).second;

    names.push_back(proc->getName());
  }
}

CTclScope *
CTclScope::
getNamedScope(const std::string &name, bool create_it)
{
  ScopeMap::iterator p = scopeMap_.find(name);

  if (p != scopeMap_.end())
    return (*p).second;

  if (! create_it)
    return NULL;

  CTclScope *scope = new CTclScope(tcl_, this, name);

  scopeMap_[name] = scope;

  return scope;
}

//-----------

uint CTclVariable::notifyProcId_;

CTclVariable::
CTclVariable(CTclValueRef value)
{
  if (value.isValid())
    value_ = CTclValueRef(value->dup());
  else
    value_ = CTclValueRef();
}

CTclVariable::
~CTclVariable()
{
}

std::string
CTclVariable::
toString() const
{
  if (hasValue())
    return getValue()->toString();

  return "";
}

bool
CTclVariable::
toBool() const
{
  if (hasValue())
    return getValue()->toBool();

  return false;
}

bool
CTclVariable::
hasValue() const
{
  return value_.isValid();
}

CTclValueRef
CTclVariable::
getValue() const
{
  return value_;
}

void
CTclVariable::
setValue(CTclValueRef value)
{
  if (value.isValid())
    value_ = CTclValueRef(value->dup());
  else
    value_ = CTclValueRef();

  callNotifyProcs();
}

CTclValueRef
CTclVariable::
getArrayValue(const std::string &indexStr) const
{
  if (! hasValue())
    return CTclValueRef();

  if (value_->getType() == CTclValue::ARRAY_TYPE)
    return value_.cast<CTclArray>()->getValue(indexStr);
  else
    return CTclValueRef();
}

void
CTclVariable::
setArrayValue(const std::string &indexStr, CTclValueRef value)
{
  if (value_->getType() == CTclValue::ARRAY_TYPE)
    value_.cast<CTclArray>()->setValue(indexStr, value);

  callNotifyProcs();
}

void
CTclVariable::
appendValue(CTclValueRef value)
{
  if (hasValue()) {
    if      (value_->getType() == CTclValue::STRING_TYPE)
      value_.cast<CTclString>()->appendValue(value->toString());
    else if (value_->getType() == CTclValue::LIST_TYPE)
      value_.cast<CTclList>()->addValue(value);
    else
      setValue(value);
  }
  else
    setValue(value);
}

uint
CTclVariable::
addNotifyProc(CTclVariableProc *proc)
{
  ++notifyProcId_;

  proc->setId(notifyProcId_);

  notifyProcs_.push_back(proc);

  return notifyProcId_;
}

void
CTclVariable::
callNotifyProcs()
{
  VariableProcList::const_iterator p1, p2;

  for (p1 = notifyProcs_.begin(), p2 = notifyProcs_.end(); p1 != p2; ++p1)
    (*p1)->notify(this);
}

//----------

CTclEnvVariable::
CTclEnvVariable()
{
}

CTclValueRef
CTclEnvVariable::
getArrayValue(const std::string &indexStr) const
{
  std::string value;

  if (! CEnvInst.get(indexStr, value))
    value = "";

  return CTclValueRef(new CTclString(value));
}

//----------

void
CTclString::
print(std::ostream &os) const
{
  os << str_;
}

std::string
CTclString::
toString() const
{
  return str_;
}

bool
CTclString::
toInt(int &i) const
{
  i = 0;

  long l = 0;

  if (! CStrUtil::toInteger(str_, &l))
    return false;

  i = int(l);

  return true;
}

bool
CTclString::
toReal(double &r) const
{
  r = 0.0;

  if (! CStrUtil::toReal(str_, &r))
    return false;

  return true;
}

bool
CTclString::
toBool() const
{
  bool b;

  if (! CStrUtil::toBool(str_, &b))
    b = false;

  return b;
}

CTclValueRef
CTclString::
toList(CTcl *tcl) const
{
  std::vector<CTclValueRef> args;

  CTclList *list = tcl->stringToList(str_);

  return CTclValueRef(list);
}

//----------

std::string
CTclArray::
toString() const
{
  return "";
}

void
CTclArray::
print(std::ostream &os) const
{
  ValueMap::const_iterator p1, p2;

  os << "{";

  for (p1 = values_.begin(), p2 = values_.end(); p1 != p2; ++p1) {
    os << " " << (*p1).first << "=";

    (*p1).second->print(os);
  }

  os << "}";
}

bool
CTclArray::
toBool() const
{
  return ! values_.empty();
}

//----------

bool
CTclList::
toBool() const
{
  return ! values_.empty();
}

std::string
CTclList::
toString() const
{
  std::string str;

  ValueList::const_iterator p1, p2;

  for (p1 = values_.begin(), p2 = values_.end(); p1 != p2; ++p1) {
    std::string str1 = (*p1)->toString();

    if (! str.empty()) str += " ";

    if (CTcl::needsBraces(str1))
      str += "{" + str1 + "}";
    else
      str += str1;
  }

  return str;
}

void
CTclList::
print(std::ostream &os) const
{
  std::string str = toString();

  os << str;
}

//----------

bool
CTclValue::
checkInt(CTcl *tcl, int &i)
{
  bool ok = toInt(i);

  if (! ok)
    tcl->badInteger(CTclValueRef(this));

  return ok;
}

bool
CTclValue::
checkReal(CTcl *tcl, double &r)
{
  bool ok = toReal(r);

  if (! ok)
    tcl->badReal(CTclValueRef(this));

  return ok;
}

bool
CTclValue::
toIndex(CTcl *tcl, int &ind)
{
  bool ok = toInt(ind);

  if (! ok) {
    const std::string &str = toString();

    if (str == "end") {
      ind = -1;
    }
    else if (str.size() > 4 && str.substr(0, 4) == "end-") {
      std::string str1 = str.substr(4);

      long ind1;

      bool ok1 = CStrUtil::toInteger(str1, &ind1);

      if (ok1)
        ind = -ind1 - 1;
    }
    else {
      tcl->throwError("bad index \"" + toString() + "\": must be integer or end?-integer?");
      return false;
    }
  }

  return true;
}

CTclValueRef
CTclValue::
eval(CTcl *tcl) const
{
  const std::string &str = toString();

  return tcl->evalString(str);
}

bool
CTclValue::
evalBool(CTcl *tcl) const
{
  CTclValueRef value = eval(tcl);

  return value->toBool();
}

CTclValueRef
CTclValue::
exec(CTcl *tcl) const
{
  const std::string &str = toString();

  return tcl->parseString(str);
}

//----------

CTclValueRef
CTclCommentCommand::
exec(const std::vector<CTclValueRef> &)
{
  return CTclValueRef();
}

//----------

CTclValueRef
CTclAfterCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("after option ?arg arg ...?");
    return CTclValueRef();
  }

  const std::string &opt = args[0]->toString();

  if      (opt == "cancel") {
    if (numArgs < 2) {
      tcl_->wrongNumArgs("after cancel id|command");
      return CTclValueRef();
    }

    for (uint i = 1; i < numArgs; ++i) {
      const std::string &id = args[i]->toString();

      tcl_->cancelTimer(id);
    }
  }
  else if (opt == "idle") {
    if (numArgs < 2) {
      tcl_->wrongNumArgs("after idle script script ...");
      return CTclValueRef();
    }

    std::string str;

    for (uint i = 1; i < numArgs; ++i) {
      if (i > 1) str += " ";

      str += args[i]->toString();
    }

    std::string name = tcl_->addTimer(0, str);

    return tcl_->createValue(name);
  }
  else if (opt == "info") {
    if (numArgs == 1) {
      std::vector<std::string> names;

      tcl_->getTimerNames(names);

      return tcl_->createValue(names);
    }
    else {
      std::vector<CTclValueRef> values;

      for (uint i = 1; i < numArgs; ++i) {
        const std::string &id = args[i]->toString();

        CTclTimer *timer = tcl_->getTimer(id);

        if (timer) {
          std::vector<CTclValueRef> values1;

          values1.push_back(tcl_->createValue(timer->getName  ()));
          values1.push_back(tcl_->createValue(timer->getScript()));

          values.push_back(tcl_->createValue(values1));
        }
        else
          tcl_->throwError("event \"" + id + "\" doesn't exist");
      }

      return tcl_->createValue(values);
    }
  }
  else {
    int ms;

    bool ok = args[0]->toInt(ms);

    if (! ok) {
      tcl_->throwError("bad argument \"" + opt + "\": must be "
                       "cancel, idle, info, or a number");
      return CTclValueRef();
    }

    if (numArgs == 1)
      COSTimer::milli_sleep(ms);
    else {
      std::string str;

      for (uint i = 1; i < numArgs; ++i) {
        if (i > 1) str += " ";

        str += args[i]->toString();
      }

      std::string name = tcl_->addTimer(ms, str);

      return tcl_->createValue(name);
    }
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclAppendCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 2) {
    tcl_->wrongNumArgs("append varName ?value value ...?");
    return CTclValueRef();
  }

  CTclScope *scope = tcl_->getScope();

  const std::string &varName = args[0]->toString();

  CTclVariableRef var = scope->getVariable(varName);

  if (! var.isValid()) {
    scope->setVariableValue(varName, args[1]);

    var = scope->getVariable(varName);

    for (uint i = 2; i < numArgs; ++i)
      var->appendValue(args[i]);
  }
  else {
    for (uint i = 1; i < numArgs; ++i)
      var->appendValue(args[i]);
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclArrayCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 2) {
    tcl_->wrongNumArgs("array option arrayName ?arg ...?");
    return CTclValueRef();
  }

  const std::string &cmd     = args[0]->toString();
  const std::string &varName = args[1]->toString();

  if      (cmd == "anymore") {
    return CTclValueRef();
  }
  else if (cmd == "donesearch") {
    return CTclValueRef();
  }
  else if (cmd == "exists") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("array exists arrayName");
      return CTclValueRef();
    }

    CTclScope *scope = tcl_->getScope();

    CTclValueRef value = scope->getVariableValue(varName);

    bool is_array = (value.isValid() && value->getType() == CTclValue::VALUE_MAP_TYPE);

    return CTclValueRef(tcl_->createValue(is_array));
  }
  else if (cmd == "get") {
    CTclScope *scope = tcl_->getScope();

    CTclValueRef value = scope->getVariableValue(varName);

    if (! value.isValid())
      return CTclValueRef();

    if (value->getType() == CTclValue::ARRAY_TYPE) {
      CTclArray *array = value.cast<CTclArray>();

      std::vector<std::string>  names;
      std::vector<CTclValueRef> values;

      array->getNameValues(names, values);

      uint num_names = names.size();

      CTclList *list = new CTclList;

      for (uint i = 0; i < num_names; ++i) {
        CTclList *list1 = new CTclList;

        list1->addValue(tcl_->createValue(names[i]));
        list1->addValue(values[i]);

        list->addValue(CTclValueRef(list1));
      }

      return CTclValueRef(list);
    }
    else
      return CTclValueRef();
  }
  else if (cmd == "names") {
    CTclScope *scope = tcl_->getScope();

    CTclValueRef value = scope->getVariableValue(varName);

    if (! value.isValid())
      return CTclValueRef();

    if (value->getType() == CTclValue::VALUE_MAP_TYPE) {
      CTclArray *array = value.cast<CTclArray>();

      std::vector<std::string> names;

      array->getNames(names);

      return CTclValueRef(tcl_->createValue(names));
    }
    else
      return CTclValueRef();
  }
  else if (cmd == "nextelement") {
    return CTclValueRef();
  }
  else if (cmd == "set") {
    if (numArgs < 3) {
      tcl_->wrongNumArgs("array set arrayName list");
      return CTclValueRef();
    }

    CTclValueRef list;

    if (args[2]->getType() == CTclValue::LIST_TYPE)
      list = args[2];
    else
      list = args[2]->toList(tcl_);

    uint length = list->getLength();

    if (length & 1) {
      tcl_->throwError("list must have an even number of elements");
      return CTclValueRef();
    }

    CTclArray *array = new CTclArray;

    for (uint i = 0; i < length; i += 2) {
      CTclValueRef name  = list->getIndexValue(i    );
      CTclValueRef value = list->getIndexValue(i + 1);

      array->setValue(name->toString(), value);
    }

    CTclScope *scope = tcl_->getScope();

    scope->setVariableValue(varName, CTclValueRef(array));

    return CTclValueRef();
  }
  else if (cmd == "size") {
    return CTclValueRef();
  }
  else if (cmd == "startsearch") {
    return CTclValueRef();
  }
  else if (cmd == "statistics") {
    return CTclValueRef();
  }
  else if (cmd == "unset") {
    tcl_->removeVariable(varName);

    return CTclValueRef();
  }
  else {
    tcl_->throwError("bad option \"" + cmd + "\": must be "
                     "anymore, donesearch, exists, get, names, nextelement, set, size, "
                     "startsearch, statistics, or unset");
    return CTclValueRef();
  }
}

//----------

CTclValueRef
CTclBreakCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 0) {
    tcl_->wrongNumArgs("break");
    return CTclValueRef();
  }

  CTclCommand *cmd = tcl_->getCommand();

  if (cmd->getType() & CTclCommand::COMMAND_TYPE_ITERATION) {
    tcl_->throwError("invoked \"continue\" outside of a loop");
    return CTclValueRef();
  }

  tcl_->setBreakFlag(true);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclCatchCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("catch command ?varName?");
    return CTclValueRef();
  }

  std::string errMsg;
  bool        isError = false;

  try {
    args[0]->exec(tcl_);
  }
  catch (CTclError err) {
    errMsg  = err.getMsg();
    isError = true;
  }

  if (numArgs > 1) {
    const std::string &varName = args[1]->toString();

    if (isError) {
      CTclScope *scope = tcl_->getScope();

      scope->setVariableValue(varName, tcl_->createValue(errMsg));
    }
  }

  return CTclValueRef(tcl_->createValue(! isError));
}

//----------

CTclValueRef
CTclCDCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  std::string dirName;

  if      (numArgs == 0)
    dirName = COSUser::getUserHome();
  else if (numArgs == 1)
    dirName = args[0]->toString();
  else {
    tcl_->wrongNumArgs("cd ?dirName?");
    return CTclValueRef();
  }

  if (! COSFile::changeDir(dirName)) {
    tcl_->throwError("couldn't change working directory to \"" + dirName + "\": "
                    "no such file or directory");
    return CTclValueRef();
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclClockCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 1) {
    tcl_->wrongNumArgs("clock option ?arg ...?");
    return CTclValueRef();
  }

  const std::string &opt = args[0]->toString();

  if      (opt == "clicks") {
    int secs, usecs;

    COSTime::getHRTime(&secs, &usecs);

    std::string str = CStrUtil::strprintf("%d%06d", secs, usecs);

    return CTclValueRef(tcl_->createValue(str));
  }
  else if (opt == "format") {
  }
  else if (opt == "scan") {
  }
  else if (opt == "seconds") {
    time_t t = time(NULL);

    return CTclValueRef(tcl_->createValue((int) t));
  }
  else {
    tcl_->throwError("bad option \"" + opt + "\": must be clicks, format, scan, or seconds");
    return CTclValueRef();
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclCloseCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 1) {
    tcl_->wrongNumArgs("close channelId");
    return CTclValueRef();
  }

  const std::string &name = args[0]->toString();

  tcl_->closeFile(name);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclContinueCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 0) {
    tcl_->wrongNumArgs("continue");
    return CTclValueRef();
  }

  CTclCommand *cmd = tcl_->getCommand();

  if (cmd->getType() & CTclCommand::COMMAND_TYPE_ITERATION) {
    tcl_->throwError("invoked \"continue\" outside of a loop");
    return CTclValueRef();
  }

  tcl_->setContinueFlag(true);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclEchoCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  for (uint i = 0; i < numArgs; ++i) {
    args[i]->print(std::cout);

    std::cout << std::endl;
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclEofCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 1) {
    tcl_->wrongNumArgs("eof channelId");
    return CTclValueRef();
  }

  const std::string &fileId = args[0]->toString();

  FILE *fp = NULL;

  if (fileId == "stdin")
    fp = stdin;
  else {
    fp = tcl_->getFile(fileId);

    if (fp == NULL) {
      tcl_->throwError("can not find channel named \"" + fileId + "\"");
      return CTclValueRef();
    }
  }

  bool flag = feof(fp);

  return CTclValueRef(tcl_->createValue(flag));
}

//----------

CTclValueRef
CTclEvalCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) {
    tcl_->wrongNumArgs("eval arg ?arg ...?");
    return CTclValueRef();
  }

  std::string str;

  for (uint i = 0; i < numArgs; ++i) {
    if (i > 0) str += " ";

    str += args[i]->toString();
  }

  return tcl_->parseString(str);
}

//----------

CTclValueRef
CTclExecCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) {
    tcl_->wrongNumArgs("exec ?switches? arg ?arg ...?");
    return CTclValueRef();
  }

  std::vector<CCommand *> cmds;

  bool ignore_arg = false;

  std::string name = args[0]->toString();
  std::string path = tcl_->lookupPathCommand(name);

  if (path == "") {
    tcl_->throwError("couldn't execute \"" + name + "\" no such file or directory");
    return CTclValueRef();
  }

  CCommand *command = new CCommand(name, path);

  cmds.push_back(command);

//CCommandMgrInst->setDebug(true);

  for (uint i = 1; i < numArgs; ++i) {
    std::string arg = args[i]->toString();

    if (ignore_arg) {
      command->addArg(arg);

      continue;
    }

    if      (arg == "--") {
      ignore_arg = true;
    }
    else if (arg == "|" || arg == "|&") {
      if (i >= numArgs - 1) {
        tcl_->throwError("illegal use of | or |& in command");
        return CTclValueRef();
      }

      ignore_arg = false;

      ++i;

      command->addPipeDest(1);

      if (arg == "|&")
        command->addPipeDest(2);

      name = args[i]->toString();
      path = tcl_->lookupPathCommand(name);

      if (path == "") {
        tcl_->throwError("couldn't execute \"" + name + "\" no such file or directory");
        return CTclValueRef();
      }

      command = new CCommand(name, path);

      cmds.push_back(command);

      command->addPipeSrc();
    }
    else if (arg == "<") {
      if (i >= numArgs - 1) {
        tcl_->throwError("can't specify \"" + arg + "\" as last word in command");
        return CTclValueRef();
      }

      ++i;

      const std::string &src = args[i]->toString();

      command->addFileSrc(src);
    }
    else if (arg == "<@") {
    }
    else if (arg == "<<") {
    }
    else if (arg == ">" || arg == ">&") {
      if (i >= numArgs - 1) {
        tcl_->throwError("can't specify \"" + arg + "\" as last word in command");
        return CTclValueRef();
      }

      ++i;

      const std::string &dest = args[i]->toString();

      command->addFileDest(dest);

      if (arg == ">&")
        command->addFileDest(dest, 2);
    }
    else if (arg == "2>") {
    }
    else if (arg == ">>") {
      if (i >= numArgs - 1) {
        tcl_->throwError("can't specify \"" + arg + "\" as last word in command");
        return CTclValueRef();
      }

      ++i;

      const std::string &dest = args[i]->toString();

      command->addFileDest(dest);

      command->setFileDestAppend(true);
    }
    else if (arg == "2>>") {
    }
    else if (arg == ">>&") {
      if (i >= numArgs - 1) {
        tcl_->throwError("can't specify \"" + arg + "\" as last word in command");
        return CTclValueRef();
      }

      ++i;

      const std::string &dest = args[i]->toString();

      command->addFileDest(dest, 1);
      command->addFileDest(dest, 2);

      command->setFileDestAppend(true);
    }
    else if (arg == ">@") {
    }
    else if (arg == "2>@") {
    }
    else if (arg == "2>@1") {
    }
    else if (arg == ">&@") {
    }
    else
      command->addArg(arg);
  }

  std::string dest;

  uint num_cmds = cmds.size();

  CCommand *cmd2 = cmds[num_cmds - 1];

  cmd2->addStringDest(dest);

  for (uint i = 0; i < num_cmds; ++i) {
    CCommand *cmd = cmds[i];

    if (i == 0)
      cmd->setProcessGroupLeader();
    else
      cmd->setProcessGroup(cmds[0]);
  }

  for (uint i = 0; i < num_cmds; ++i) {
    CCommand *cmd = cmds[i];

    cmd->start();
  }

  for (uint i = 0; i < num_cmds; ++i) {
    CCommand *cmd = cmds[i];

    cmd->wait();
  }

  dest = CStrUtil::stripSpaces(dest);

  for (uint i = 0; i < num_cmds; ++i)
    delete cmds[i];

  return CTclValueRef(tcl_->createValue(dest));
}

//----------

CTclValueRef
CTclExitCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs > 1) {
    tcl_->wrongNumArgs("exit ?returnCode?");
    return CTclValueRef();
  }

  int code = 0;

  if (numArgs > 0) {
    if (! args[0]->checkInt(tcl_, code))
      return CTclValueRef();
  }

  exit(code);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclExprCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  std::string str;

  for (uint i = 0; i < numArgs; ++i) {
    if (i > 0) str += " ";

    str += args[i]->toString();
  }

  return tcl_->evalString(str);
}

//----------

CTclValueRef
CTclFileCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) {
    tcl_->wrongNumArgs("file option ?arg ...?");
    return CTclValueRef();
  }

  const std::string &option = args[0]->toString();

  if      (option == "atime") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file atime name ?time?");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    int atime = CFile::getATime(fileName);

    return CTclValueRef(tcl_->createValue(atime));
  }
  else if (option == "attributes") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file attributes name ?option? ?value? ?option value ...?");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    struct stat file_stat;

    if (! CFile::getStat(fileName, &file_stat)) {
      tcl_->wrongNumArgs("could not read \"" + fileName + "\": no such file or directory");
      return CTclValueRef();
    }

    std::string str;

    str += "-group " + COSUser::getGroupName(file_stat.st_gid);
    str += " -owner " + COSUser::getUserName(file_stat.st_uid);
    str += " -permissions 0" + CStrUtil::toOctString(file_stat.st_mode);

    return CTclValueRef(tcl_->createValue(str));
  }
  else if (option == "channels") {
    CTclList *list = new CTclList;

    if (numArgs == 1) {
      list->addValue(tcl_->createValue("stdin"));
      list->addValue(tcl_->createValue("stdout"));
      list->addValue(tcl_->createValue("stderr"));
    }

    return CTclValueRef(list);
  }
  else if (option == "copy") {
  }
  else if (option == "delete") {
  }
  else if (option == "dirname") {
  }
  else if (option == "executable") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file isdirectory name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    bool is_exec = CFile::isExecutable(fileName);

    return CTclValueRef(tcl_->createValue(is_exec));
  }
  else if (option == "exists") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file exists name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    bool exists = CFile::exists(fileName);

    return CTclValueRef(tcl_->createValue(exists));
  }
  else if (option == "extension") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file isdirectory name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    std::string suffix = CFile::getSuffix(fileName);

    if (! suffix.empty())
      suffix = "." + suffix;

    return CTclValueRef(tcl_->createValue(suffix));
  }
  else if (option == "isdirectory") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file isdirectory name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    bool is_dir = CFile::isDirectory(fileName);

    return CTclValueRef(tcl_->createValue(is_dir));
  }
  else if (option == "isfile") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file isregular name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    bool is_dir = CFile::isRegular(fileName);

    return CTclValueRef(tcl_->createValue(is_dir));
  }
  else if (option == "join") {
  }
  else if (option == "link") {
  }
  else if (option == "lstat") {
  }
  else if (option == "mkdir") {
  }
  else if (option == "mtime") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file atime name ?time?");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    int mtime = CFile::getMTime(fileName);

    return CTclValueRef(tcl_->createValue(mtime));
  }
  else if (option == "nativename") {
  }
  else if (option == "normalize") {
  }
  else if (option == "owned") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file isregular name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    bool is_owner = CFile::isOwner(fileName);

    return CTclValueRef(tcl_->createValue(is_owner));
  }
  else if (option == "pathtype") {
  }
  else if (option == "readable") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file isdirectory name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    bool is_read = CFile::isReadable(fileName);

    return CTclValueRef(tcl_->createValue(is_read));
  }
  else if (option == "readlink") {
  }
  else if (option == "rename") {
  }
  else if (option == "rootname") {
  }
  else if (option == "separator") {
  }
  else if (option == "size") {
  }
  else if (option == "split") {
  }
  else if (option == "stat") {
  }
  else if (option == "system") {
  }
  else if (option == "tail") {
  }
  else if (option == "type") {
  }
  else if (option == "volumes") {
  }
  else if (option == "writable") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("file isdirectory name");
      return CTclValueRef();
    }

    const std::string &fileName = args[1]->toString();

    bool is_write = CFile::isWritable(fileName);

    return CTclValueRef(tcl_->createValue(is_write));
  }
  else {
    tcl_->throwError("bad option \"" + option + "\": must be "
                     "atime, attributes, channels, copy, delete, dirname, executable, "
                     "exists, extension, isdirectory, isfile, join, link, lstat, mtime, "
                     "mkdir, nativename, normalize, owned, pathtype, readable, readlink, "
                     "rename, rootname, separator, size, split, stat, system, tail, type, "
                     "volumes, or writable");
    return CTclValueRef();
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclFlushCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 1) {
    tcl_->wrongNumArgs("flush channelId");
    return CTclValueRef();
  }

  FILE *fp = NULL;

  const std::string &fileId = args[0]->toString();

  if      (fileId == "stdout")
    fp = stdout;
  else if (fileId == "stderr")
    fp = stderr;
  else {
    fp = tcl_->getFile(fileId);

    if (fp == NULL) {
      tcl_->throwError("can not find channel named \"" + fileId + "\"");
      return CTclValueRef();
    }
  }

  fflush(fp);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclForCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 4) {
    tcl_->wrongNumArgs("for start test next command");
    return CTclValueRef();
  }

  tcl_->setBreakFlag   (false);
  tcl_->setContinueFlag(false);

  args[0]->exec(tcl_);

  while (args[1]->evalBool(tcl_)) {
    args[3]->exec(tcl_);

    tcl_->setContinueFlag(false);

    args[2]->exec(tcl_);

    if (tcl_->getBreakFlag() || tcl_->getReturnFlag()) break;
  }

  tcl_->setBreakFlag   (false);
  tcl_->setContinueFlag(false);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclForeachCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 3) {
    tcl_->wrongNumArgs("foreach varList list ?varList list ...? command");
    return CTclValueRef();
  }

  CTclValueRef varList;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    varList = args[0];
  else
    varList = args[0]->toList(tcl_);

  uint numVars = varList->getLength();

  if (numVars <= 0) {
    tcl_->throwError("foreach varlist is empty");
    return CTclValueRef();
  }

  CTclValueRef valList;

  if (args[1]->getType() == CTclValue::LIST_TYPE)
    valList = args[1];
  else
    valList = args[1]->toList(tcl_);

  uint numVals = valList->getLength();

  CTclScope *scope = tcl_->getScope();

  tcl_->setBreakFlag   (false);
  tcl_->setContinueFlag(false);

  uint numIters = numVals/numVars;

  for (uint i = 0, k = 0; i < numIters; ++i, k += numVars) {
    for (uint j = 0; j < numVars; ++j) {
      CTclValueRef value = valList->getIndexValue(k + j);

      const std::string &varName = varList->getIndexValue(j)->toString();

      scope->setVariableValue(varName, value);
    }

    tcl_->setContinueFlag(false);

    args[2]->exec(tcl_);

    if (tcl_->getBreakFlag() || tcl_->getReturnFlag()) break;
  }

  tcl_->setBreakFlag   (false);
  tcl_->setContinueFlag(false);

  return CTclValueRef();
}

//----------

class CTclPrintF : public CPrintF {
 public:
  CTclPrintF(const std::string &fmt, const std::vector<CTclValueRef> &args) :
   CPrintF(fmt), args_(args), argNum_(0), numArgs_(0) {
    numArgs_ = args_.size();
  }

  CTclValueRef nextArg() const { return args_[argNum_++]; }

  int   getInt     () const { int i=0; if (argNum_ < numArgs_) nextArg()->toInt(i); return i; }
  long  getLong    () const { int i=0; if (argNum_ < numArgs_) nextArg()->toInt(i); return i; }
  LLong getLongLong() const { int i=0; if (argNum_ < numArgs_) nextArg()->toInt(i); return i; }

  double getDouble() const { double r=0; if (argNum_ < numArgs_) nextArg()->toReal(r); return r; }

  std::string getString() const { return (argNum_ < numArgs_ ? nextArg()->toString() : "" ); }

 private:
  std::vector<CTclValueRef> args_;
  mutable uint              argNum_;
  uint                      numArgs_;
};

CTclValueRef
CTclFormatCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("format formatString ?arg arg ...?");
    return CTclValueRef();
  }

  const std::string &fmt = args[0]->toString();

  CTclPrintF printf(fmt, args);

  printf.nextArg();

  std::string str = printf.format();

  return CTclValueRef(tcl_->createValue(str));
}

//----------

CTclValueRef
CTclGetsCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1 || numArgs > 2) {
    tcl_->wrongNumArgs("gets channelId ?varName?");
    return CTclValueRef();
  }

  const std::string &fileId = args[0]->toString();

  FILE *fp = NULL;

  if (fileId == "stdin")
    fp = stdin;
  else {
    fp = tcl_->getFile(fileId);

    if (fp == NULL) {
      tcl_->throwError("can not find channel named \"" + fileId + "\"");
      return CTclValueRef();
    }
  }

  std::string line;

  int c = getc(fp);

  while (c != EOF && c != '\n') {
    line += (char) c;

    c = getc(fp);
  }

  if (numArgs == 2) {
    const std::string &varName = args[1]->toString();

    CTclScope *scope = tcl_->getScope();

    scope->setVariableValue(varName, tcl_->createValue(line));

    uint len = line.size();

    return CTclValueRef(tcl_->createValue(len));
  }
  else
    return CTclValueRef(tcl_->createValue(line));
}

//----------

CTclValueRef
CTclGlobCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("glob ?switches? name ?name ...?");
    return CTclValueRef();
  }

  bool        join       = true;
  bool        complain   = false;
  bool        tails      = true;
  std::string directory  = "";
  std::string pathPrefix = "";
  std::string types      = "";

  uint pos = 0;

  while (pos < numArgs) {
    const std::string &arg = args[pos]->toString();

    if (arg.size() > 0 && arg[0] == '-') {
      ++pos;

      if      (arg == "--"         ) break;
      else if (arg == "-join"      ) join     = true;
      else if (arg == "-nocomplain") complain = false;
      else if (arg == "-tails"     ) tails    = true;

      else if (arg == "-directory") {
        if (pos < numArgs)
          directory = args[pos++]->toString();
      }
      else if (arg == "-path") {
        if (pos < numArgs)
          pathPrefix = args[pos++]->toString();
      }
      else if (arg == "-types") {
        if (pos < numArgs)
          types = args[pos++]->toString();
      }
      else {
        tcl_->throwError("bad option \"" + arg + "\": must be -directory, -join, "
                         "-nocomplain, -path, -tails, -types, or --");
        return CTclValueRef();
      }
    }
    else
      break;
  }

  std::string allArgs;

  for (uint pos1 = pos; pos1 < numArgs; ++pos1) {
    const std::string &arg = args[pos]->toString();

    if (pos1 > pos) allArgs += " ";

    allArgs += arg;
  }

  if (directory != "") {
    if (! CDir::enter(directory)) {
      if (complain) tcl_->throwError("no files matched glob patterns \"" + allArgs + "\"");
      return CTclValueRef();
    }
  }

  std::vector<std::string> files;

  for ( ; pos < numArgs; ++pos) {
    const std::string &arg = args[pos]->toString();

    CFileMatch match;

    std::vector<std::string> files1;

    match.matchCurrentDir(arg, files1);

    std::copy(files1.begin(), files1.end(), std::back_inserter(files));
  }

  if (directory != "")
    CDir::leave();

  if (files.empty()) {
    if (complain) tcl_->throwError("no files matched glob patterns \"" + allArgs + "\"");
    return CTclValueRef();
  }

  return CTclValueRef(tcl_->createValue(files));
}

//----------

CTclValueRef
CTclGlobalCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("global varName ?varName ...?");
    return CTclValueRef();
  }

  CTclScope *scope  = tcl_->getScope();
  CTclScope *gscope = tcl_->getGlobalScope();

  for (uint i = 0; i < numArgs; ++i) {
    const std::string &varName = args[i]->toString();

    CTclVariableRef var = scope->getVariable(varName);

    if (var.isValid()) {
      tcl_->throwError("variable \"" + varName + "\" already exists");
      return CTclValueRef();
    }

    var = gscope->getVariable(varName);

    if (! var.isValid())
      var = gscope->addVariable(varName, CTclValueRef());

    scope->addVariable(varName, var);
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclHistoryCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  std::string cmd;

  if (numArgs > 0) {
    const std::string &cmd = args[0]->toString();

    if      (cmd == "add") {
    }
    else if (cmd == "change") {
    }
    else if (cmd == "clear") {
    }
    else if (cmd == "event") {
    }
    else if (cmd == "info") {
    }
    else if (cmd == "keep") {
    }
    else if (cmd == "nextid") {
    }
    else if (cmd == "redo") {
    }
    else {
      tcl_->throwError("bad option \"" + cmd + "\": must be "
                       "add, change, clear, event, info, keep, nextid, or redo");
      return CTclValueRef();
    }
  }
  else {
    CHistory *history = tcl_->getHistory();

    CHistory::CommandList::const_iterator pc1, pc2;

    for (pc1 = history->beginCommand(), pc2 = history->endCommand(); pc1 != pc2; ++pc1) {
      CHistoryCommand *cmd = *pc1;

      std::cout << cmd->getNumber() << " " << cmd->getCommand() << std::endl;
    }
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclIfCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 2) {
    tcl_->throwError("wrong # args: no expression after \"if\" argument");
    return CTclValueRef();
  }

  bool has_then = false;

  const std::string &name = args[1]->toString();

  if (name == "then") {
    if (numArgs < 3) {
      tcl_->throwError("wrong # args: no script following \"then\" argument");
      return CTclValueRef();
    }

    has_then = true;
  }

  if (args[0]->evalBool(tcl_)) {
    int pos = (has_then ? 2 : 1);

    args[pos]->exec(tcl_);

    return CTclValueRef();
  }

  uint pos = (has_then ? 3 : 2);

  while (pos < numArgs) {
    const std::string &name = args[pos]->toString();

    if      (name == "elseif") {
      ++pos;

      if  (pos >= numArgs) {
        tcl_->throwError("wrong # args: no expression following \"elseif\" argument");
        return CTclValueRef();
      }

      if (pos >= numArgs - 1) {
        const std::string &arg = args[pos]->toString();
        tcl_->throwError("wrong # args: no script following \"" + arg + "\" argument");
        return CTclValueRef();
      }

      if (args[pos]->evalBool(tcl_)) {
        args[pos + 1]->exec(tcl_);

        return CTclValueRef();
      }

      pos += 2;
    }
    else if (name == "else") {
      ++pos;

      if (pos >= numArgs) {
        tcl_->throwError("wrong # args: no script following \"else\" argument");
        return CTclValueRef();
      }

      if (pos < numArgs - 1) {
        tcl_->throwError("wrong # args: extra words after \"else\" clause in \"if\" command");
        return CTclValueRef();
      }

      args[pos]->exec(tcl_);

      return CTclValueRef();
    }
    else {
      tcl_->throwError("invalid command name \"" + name + "\"");
      return CTclValueRef();
    }
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclIncrCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1 || numArgs > 2)
    return CTclValueRef();

  const std::string &varName = args[0]->toString();

  CTclScope *scope = tcl_->getScope();

  CTclVariableRef var = scope->getVariable(varName);

  if (! var.isValid()) {
    tcl_->throwError("can't read \"" + varName + "\": no such variable");
    return CTclValueRef();
  }

  CTclValueRef value = var->getValue();

  int ivalue;

  bool ok = value->toInt(ivalue);

  if (numArgs == 1) {
    if (ok)
      ++ivalue;
  }
  else {
    int inc;

    bool ok = args[1]->toInt(inc);

    if (ok)
      ivalue += inc;
  }

  CTclValueRef value1 = tcl_->createValue(ivalue);

  var->setValue(value1);

  return value1;
}

//----------

CTclValueRef
CTclInfoCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) {
    tcl_->wrongNumArgs("info option ?arg arg ...?");
    return CTclValueRef();
  }

  const std::string &cmd = args[0]->toString();

  if      (cmd == "args") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("info args procname");
      return CTclValueRef();
    }

    const std::string &procName = args[1]->toString();

    CTclProc *proc = tcl_->getProc(procName);

    if (! proc) {
      tcl_->throwError("\"" + procName + "\" is not a procedure");
      return CTclValueRef();
    }

    std::vector<std::string> args;

    proc->getArgs(args);

    return CTclValueRef(tcl_->createValue(args));
  }
  else if (cmd == "body") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("info body procname");
      return CTclValueRef();
    }

    const std::string &procName = args[1]->toString();

    CTclProc *proc = tcl_->getProc(procName);

    if (! proc) {
      tcl_->throwError("\"" + procName + "\" is not a procedure");
      return CTclValueRef();
    }

    return proc->getBody();
  }
  else if (cmd == "cmdcount") {
    return CTclValueRef(tcl_->createValue(0));
  }
  else if (cmd == "commands") {
    std::vector<std::string> names;

    tcl_->getCommandNames(names);

    return CTclValueRef(tcl_->createValue(names));
  }
  else if (cmd == "complete") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("info complete command");
      return CTclValueRef();
    }

    const std::string &cmd = args[1]->toString();

    bool rc = tcl_->isCompleteLine(cmd);

    return CTclValueRef(tcl_->createValue(rc ? 1 : 0));
  }
  else if (cmd == "default") {
  }
  else if (cmd == "exists") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("info exists command");
      return CTclValueRef();
    }

    CTclScope *scope = tcl_->getScope();

    const std::string &varName = args[1]->toString();

    CTclVariableRef var = scope->getVariable(varName);

    return CTclValueRef(tcl_->createValue(var.isValid() ? 1 : 0));
  }
  else if (cmd == "frame") {
  }
  else if (cmd == "functions") {
  }
  else if (cmd == "globals") {
    CTclScope *scope = tcl_->getGlobalScope();

    std::vector<std::string> names;

    scope->getVariableNames(names);

    return CTclValueRef(tcl_->createValue(names));
  }
  else if (cmd == "hostname") {
    std::string name = COSUser::getHostName();

    return CTclValueRef(tcl_->createValue(name));
  }
  else if (cmd == "level") {
    return CTclValueRef(tcl_->createValue(0));
  }
  else if (cmd == "library") {
  }
  else if (cmd == "loaded") {
  }
  else if (cmd == "locals") {
  }
  else if (cmd == "nameofexecutable") {
  }
  else if (cmd == "patchlevel") {
  }
  else if (cmd == "procs") {
    std::vector<std::string> names;

    CTclScope *scope = tcl_->getGlobalScope();

    scope->getProcNames(names);

    return CTclValueRef(tcl_->createValue(names));
  }
  else if (cmd == "script") {
  }
  else if (cmd == "sharedlibextension") {
  }
  else if (cmd == "tclversion") {
  }
  else if (cmd == "vars") {
    CTclScope *scope = tcl_->getScope();

    std::vector<std::string> names;

    scope->getVariableNames(names);

    return CTclValueRef(tcl_->createValue(names));
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclJoinCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0 || numArgs > 2) {
    tcl_->wrongNumArgs("join list ?joinString?");
    return CTclValueRef();
  }

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  std::string sep = " ";

  if (numArgs == 2)
    sep = args[1]->toString();

  std::string res;

  uint length = list->getLength();

  for (uint i = 0; i < length; ++i) {
    std::string item = list->getIndexValue(i)->toString();

    if (i > 0) res += sep;

    res += item;
  }

  return CTclValueRef(tcl_->createValue(res));
}

//----------

CTclValueRef
CTclLAppendCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) {
    tcl_->wrongNumArgs("lappend varName ?value value ...?");
    return CTclValueRef();
  }

  const std::string &varName = args[0]->toString();

  CTclScope *scope = tcl_->getScope();

  CTclVariableRef var = scope->getVariable(varName);

  if (! var.isValid()) {
    CTclList *list = new CTclList;

    scope->setVariableValue(varName, CTclValueRef(list));

    var = scope->getVariable(varName);
  }

  CTclValueRef value = var->getValue();

  CTclValueRef list;

  if (value->getType() == CTclValue::LIST_TYPE)
    list = value;
  else
    list = value->toList(tcl_);

  for (uint i = 1; i < numArgs; ++i)
    list->addValue(args[i]);

  return CTclValueRef(list);
}

//----------

CTclValueRef
CTclListCommand::
exec(const std::vector<CTclValueRef> &args)
{
  CTclList *list = new CTclList;

  uint numArgs = args.size();

  for (uint i = 0; i < numArgs; ++i)
    list->addValue(args[i]);

  return CTclValueRef(list);
}

//----------

CTclValueRef
CTclLIndexCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) {
    tcl_->wrongNumArgs("lindex list ?index...?");
    return CTclValueRef();
  }

  if (numArgs == 1) {
    if (args[0]->getType() == CTclValue::LIST_TYPE)
      return args[0];
    else
      return args[0]->toList(tcl_);
  }

  int ind;

  if (! args[1]->toIndex(tcl_, ind))
    return CTclValueRef();

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  int length = list->getLength();

  if (ind < 0) ind = length + ind;

  if (ind < 0 || ind >= length)
    return CTclValueRef();

  return list->getIndexValue(ind);
}

//----------

CTclValueRef
CTclLInsertCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 3) {
    tcl_->wrongNumArgs("linsert list index element ?element ...?");
    return CTclValueRef();
  }

  int ind;

  if (! args[1]->toIndex(tcl_, ind))
    return CTclValueRef();

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  int length = list->getLength();

  if (ind < 0) ind = length + ind;

  if (ind < 0 || ind > length)
    return CTclValueRef();

  CTclList *list1 = new CTclList;

  for (int i = 0; i < ind; ++i)
    list1->addValue(list->getIndexValue(i));

  for (uint i = 2; i < numArgs; ++i)
    list1->addValue(args[i]);

  for (uint i = ind; i < numArgs; ++i)
    list1->addValue(list->getIndexValue(i));

  return CTclValueRef(list1);
}

//----------

CTclValueRef
CTclLLengthCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 1) {
    tcl_->wrongNumArgs("llength list");
    return CTclValueRef();
  }

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  uint length = list->getLength();

  return CTclValueRef(tcl_->createValue(length));
}

//----------

CTclValueRef
CTclLRangeCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 3) {
    tcl_->wrongNumArgs("lrange list first last");
    return CTclValueRef();
  }

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  int length = list->getLength();

  int first;

  if (! args[1]->toIndex(tcl_, first))
    return CTclValueRef();

  int last;

  if (! args[2]->toIndex(tcl_, last))
    return CTclValueRef();

  CTclList *list1 = new CTclList;

  for (int i = first; i <= last && i >= 0 && i < length; ++i) {
    CTclValueRef value = list->getIndexValue(i);

    list1->addValue(CTclValueRef(value->dup()));
  }

  return CTclValueRef(list1);
}

//----------

CTclValueRef
CTclLRepeatCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 2) {
    tcl_->wrongNumArgs("lrepeat positiveCount value ?value ...?");
    return CTclValueRef();
  }

  int count;

  if (! args[0]->toIndex(tcl_, count))
    return CTclValueRef();

  CTclList *list = new CTclList;

  for (int i = 0; i < count; ++i) {
    for (uint j = 1; j < numArgs; ++j)
      list->addValue(CTclValueRef(args[j]->dup()));
  }

  return CTclValueRef(list);
}

//----------

CTclValueRef
CTclLReplaceCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 3) {
    tcl_->wrongNumArgs("lreplace list first last ?element element ...?");
    return CTclValueRef();
  }

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  int ind1, ind2;

  if (! args[1]->toIndex(tcl_, ind1))
    return CTclValueRef();

  if (! args[2]->toIndex(tcl_, ind2))
    return CTclValueRef();

  int length = list->getLength();

  if (ind1 < 0) ind1 += length;
  if (ind2 < 0) ind2 += length;

  CTclList *list1 = new CTclList;

  if (ind1 < 0 || ind1 >= length) {
    tcl_->throwError("list doesn't contain element " + CStrUtil::toString(ind1));
    return CTclValueRef();
  }

  if (ind2 < 0 || ind2 >= length) {
    tcl_->throwError("list doesn't contain element " + CStrUtil::toString(ind2));
    return CTclValueRef();
  }

  for (int i = 0; i < ind1 && i < int(length); ++i) {
    CTclValueRef value = list->getIndexValue(i);

    list1->addValue(value);
  }

  for (uint i = 3; i < numArgs; ++i)
    list1->addValue(args[i]);

  for (int i = ind2 + 1; i < int(length); ++i) {
    CTclValueRef value = list->getIndexValue(i);

    list1->addValue(value);
  }

  return CTclValueRef(list1);
}

//----------

CTclValueRef
CTclLSearchCommand::
exec(const std::vector<CTclValueRef> &args)
{
#if 0
  static CommandOpts[] = {
    { "-exact"      , COMMAND_OPT_FLAG },
    { "-glob"       , COMMAND_OPT_FLAG },
    { "-regexp"     , COMMAND_OPT_FLAG },
    { "-sorted"     , COMMAND_OPT_FLAG },
    { "-all"        , COMMAND_OPT_FLAG },
    { "-inline"     , COMMAND_OPT_FLAG },
    { "-not"        , COMMAND_OPT_FLAG },
    { "-start"      , COMMAND_OPT_FLAG },
    { "-ascii"      , COMMAND_OPT_FLAG },
    { "-dictionary" , COMMAND_OPT_FLAG },
    { "-integer"    , COMMAND_OPT_FLAG },
    { "-nocase"     , COMMAND_OPT_FLAG },
    { "-real"       , COMMAND_OPT_FLAG },
    { "-decreasing" , COMMAND_OPT_FLAG },
    { "-increasing" , COMMAND_OPT_FLAG },
    { "-index"      , COMMAND_OPT_LIST },
    { "-subindices" , COMMAND_OPT_FLAG },
  };
#endif

  uint numArgs = args.size();

  if (numArgs != 2) {
    tcl_->wrongNumArgs("lsearch ?options? list pattern");
    return CTclValueRef();
  }

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  CTclValueRef value = args[1];

  uint length = list->getLength();

  for (uint i = 0; i < length; ++i) {
    CTclValueRef value1 = list->getIndexValue(i);

    if (value1->cmp(value) == 0)
      return CTclValueRef(tcl_->createValue(i));
  }

  return CTclValueRef(tcl_->createValue(-1));
}

//----------

CTclValueRef
CTclLSetCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 2) {
    tcl_->wrongNumArgs("lset listVar index ?index...? value");
    return CTclValueRef();
  }

  CTclScope *scope = tcl_->getScope();

  const std::string &varName = args[0]->toString();

  CTclVariableRef var = scope->getVariable(varName);

  if (! var.isValid()) {
    tcl_->throwError("can't read \"" + varName + "\": no such variable");
    return CTclValueRef();
  }

  if (numArgs > 2) {
    CTclValueRef value = var->getValue();

    for (uint i = 1; i < numArgs - 2; ++i) {
      CTclValueRef list;

      if (value->getType() == CTclValue::LIST_TYPE)
        list = value;
      else
        list = value->toList(tcl_);

      int ind;

      if (! args[i]->toIndex(tcl_, ind))
        return CTclValueRef();

      value = list->getIndexValue(ind);
    }

    CTclValueRef list;

    if (value->getType() == CTclValue::LIST_TYPE)
      list = value;
    else
      list = value->toList(tcl_);

    int ind;

    if (! args[numArgs - 2]->toIndex(tcl_, ind))
      return CTclValueRef();

    list->setIndexValue(ind, args[numArgs - 1]);

    var->setValue(list);
  }
  else
    var->setValue(args[1]);

  return var->getValue();
}

//----------

CTclValueRef
CTclLSortCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 1) {
    tcl_->wrongNumArgs("lsort list");
    return CTclValueRef();
  }

  CTclValueRef list;

  if (args[0]->getType() == CTclValue::LIST_TYPE)
    list = args[0];
  else
    list = args[0]->toList(tcl_);

  typedef std::set<CTclValueRef> ValueSet;

  ValueSet valueSet;

  uint length = list->getLength();

  for (uint i = 0; i < length; ++i) {
    CTclValueRef value = list->getIndexValue(i);

    valueSet.insert(value);
  }

  CTclList *list1 = new CTclList;

  ValueSet::const_iterator p1, p2;

  for (p1 = valueSet.begin(), p2 = valueSet.end(); p1 != p2; ++p1)
    list1->addValue(*p1);

  return CTclValueRef(list1);
}

//----------

CTclValueRef
CTclNamespaceCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("namespace subcommand ?arg ...?");
    return CTclValueRef();
  }

  const std::string &cmd = args[0]->toString();

  if      (cmd == "children") {
  }
  else if (cmd == "code") {
  }
  else if (cmd == "current") {
  }
  else if (cmd == "delete") {
  }
  else if (cmd == "eval") {
    if (numArgs < 3) {
      tcl_->wrongNumArgs("namespace eval name ?arg ...?");
      return CTclValueRef();
    }

    const std::string &name = args[1]->toString();

    CTclScope *scope = tcl_->getNamedScope(name, true);

    tcl_->pushScope(scope);

    for (uint i = 2; i < numArgs; ++i)
      args[i]->exec(tcl_);

    tcl_->popScope();
  }
  else if (cmd == "exists") {
  }
  else if (cmd == "export") {
  }
  else if (cmd == "forget") {
  }
  else if (cmd == "import") {
  }
  else if (cmd == "inscope") {
  }
  else if (cmd == "origin") {
  }
  else if (cmd == "parent") {
  }
  else if (cmd == "qualifiers") {
  }
  else if (cmd == "tail") {
  }
  else if (cmd == "which") {
  }
  else {
    tcl_->throwError("bad option \"" + cmd + "\": must be children, code, current, "
                     "delete, eval, exists, export, forget, import, inscope, origin, "
                     "parent, qualifiers, tail, or which");
    return CTclValueRef();
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclOpenCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("open fileName ?access? ?permissions?");
    return CTclValueRef();
  }

  const std::string &name = args[0]->toString();

  std::string access = "r";

  if (numArgs > 1)
    access = args[1]->toString();

  int perm = 0666;

  if (numArgs > 2) {
    if (! args[2]->checkInt(tcl_, perm))
      return CTclValueRef();
  }

  std::string handle = tcl_->openFile(name, access);

  return CTclValueRef(tcl_->createValue(handle));
}

//----------

CTclValueRef
CTclProcCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 3) return CTclValueRef();

  const std::string &name = args[0]->toString();

  CTclValueRef list;

  if (args[1]->getType() == CTclValue::LIST_TYPE)
    list = args[1];
  else
    list = args[1]->toList(tcl_);

  std::vector<std::string> args1;

  uint length = list->getLength();

  for (uint i = 0; i < length; ++i) {
    CTclValueRef value = list->getIndexValue(i);

    // TODO: name + def value

    args1.push_back(value->toString());
  }

  tcl_->defineProc(name, args1, args[2]);

  return CTclValueRef();
}

//----------

// puts [-nonewline] [channelId] string
CTclValueRef
CTclPutsCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  int pos = 0;

  bool newline = true;

  if (numArgs > 1) {
    if (args[pos]->toString() == "-nonewline") {
      newline = false;

      ++pos;
      --numArgs;
    }
  }

  FILE *fp = stdout;

  if (numArgs > 1) {
    const std::string &fileId = args[pos]->toString();

    if      (fileId == "stdout")
      fp = stdout;
    else if (fileId == "stderr")
      fp = stderr;
    else {
      fp = tcl_->getFile(fileId);

      if (fp == NULL) {
        tcl_->throwError("can not find channel named \"" + fileId + "\"");
        return CTclValueRef();
      }
    }

    ++pos;
    --numArgs;
  }

  if (numArgs == 1) {
    std::ostringstream ostr;

    args[pos]->print(ostr);

    const std::string &str = ostr.str();

    fwrite(str.c_str(), 1, str.size(), fp);
  }

  if (newline)
    fputc('\n', fp);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclPidCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs > 1) {
    tcl_->wrongNumArgs("pid");
    return CTclValueRef();
  }

  int pid = COSProcess::getProcessId();

  return CTclValueRef(tcl_->createValue(pid));
}

//----------

CTclValueRef
CTclPwdCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 0) {
    tcl_->wrongNumArgs("pwd");
    return CTclValueRef();
  }

  std::string dirName = COSFile::getCurrentDir();

  return CTclValueRef(tcl_->createValue(dirName));
}

//----------

CTclValueRef
CTclReadCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();
  uint pos     = 0;

  bool newline = true;

  if (numArgs > 1) {
    if (args[pos]->toString() == "-nonewline") {
      newline = false;

      ++pos;
      --numArgs;
    }
  }

  if (numArgs != 1 && numArgs != 2) {
    tcl_->wrongNumArgs("read channelId ?numChars?\" or \"read ?-nonewline? channelId");
    return CTclValueRef();
  }

  const std::string &fileId = args[pos]->toString();

  std::string dirName = COSFile::getCurrentDir();

  FILE *fp = NULL;

  if (fileId == "stdin")
    fp = stdin;
  else {
    fp = tcl_->getFile(fileId);

    if (fp == NULL) {
      tcl_->throwError("can not find channel named \"" + fileId + "\"");
      return CTclValueRef();
    }
  }

  std::string line;

  if (numArgs == 1) {
    int c = getc(fp);

    while (c != EOF) {
      line += (char) c;

      c = getc(fp);
    }
  }
  else {
    int num;

    if (! args[pos + 1]->checkInt(tcl_, num))
      return CTclValueRef();

    for (int i = 0; i < num; ++i) {
      int c = getc(fp);

      if (c == EOF) break;

      line += (char) c;
    }
  }

  return CTclValueRef(tcl_->createValue(line));
}

//----------

CTclValueRef
CTclReturnCommand::
exec(const std::vector<CTclValueRef> &args)
{
  CTclValueRef retVal;

  uint numArgs = args.size();

  if (numArgs > 0)
    retVal = args[0];

  tcl_->setReturnFlag(retVal);

  return CTclValueRef();
}

//-----------

CTclValueRef
CTclSetCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1 || numArgs > 2)
    return CTclValueRef();

  const std::string &varName = args[0]->toString();

  CTclScope *scope = tcl_->getScope();

  std::string::size_type pos1 = varName.find ('(');
  std::string::size_type pos2 = varName.rfind(')');

  if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 == varName.size() - 1) {
    std::string varName1 = varName.substr(0, pos1);
    std::string indexStr = varName.substr(pos1 + 1, pos2 - pos1 - 1);

    if (numArgs == 2) {
      scope->setArrayVariableValue(varName1, indexStr, args[1]);

      return args[1];
    }
    else
      return scope->getArrayVariableValue(varName1, indexStr);
  }
  else {
    if (numArgs == 2) {
      scope->setVariableValue(varName, args[1]);

      return args[1];
    }
    else
      return scope->getVariableValue(varName);
  }

  return CTclValueRef();
}

//-----------

CTclValueRef
CTclSourceCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 1) {
    tcl_->wrongNumArgs("source fileName");
    return CTclValueRef();
  }

  const std::string &fileName = args[0]->toString();

  tcl_->parseFile(fileName);

  return CTclValueRef();
}

//-----------

CTclValueRef
CTclStringCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs == 0) {
    tcl_->wrongNumArgs("string option arg ?arg ...?");
    return CTclValueRef();
  }

  const std::string &cmd = args[0]->toString();

  if      (cmd == "bytelength") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("wrong # args: should be \"string bytelength string\"");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    uint len = str.size();

    return CTclValueRef(tcl_->createValue(len));
  }
  else if (cmd == "compare" || cmd == "equal") {
    if (numArgs < 3) {
      tcl_->wrongNumArgs("string " + cmd + " ?-nocase? ?-length int? string1 string2");
      return CTclValueRef();
    }

    bool is_equal = (cmd == "equal");

    bool        nocase = false;
    int         length = -1;
    std::string str1, str2;

    uint pos = 1;

    if (pos < numArgs && args[pos]->toString() == "-nocase") {
      ++pos;

      nocase = true;
    }

    if (pos < numArgs && args[pos]->toString() == "-length") {
      ++pos;

      if (pos < numArgs) {
        bool ok = args[pos++]->toInt(length);

        if (! ok) return CTclValueRef();
      }
    }

    if (pos < numArgs)
      str1 = args[pos++]->toString();

    if (pos < numArgs)
      str2 = args[pos++]->toString();

    int cmp = 0;

    if (length >= 0) {
      str1 = str1.substr(0, length);
      str2 = str2.substr(0, length);

      if (nocase)
        cmp = CStrUtil::casecmp(str1, str2);
      else
        cmp = CStrUtil::cmp(str1, str2);
    }
    else {
      if (nocase)
        cmp = CStrUtil::casecmp(str1, str2);
      else
        cmp = CStrUtil::cmp(str1, str2);
    }

    if (is_equal)
      return CTclValueRef(tcl_->createValue(cmp));
    else
      return CTclValueRef(tcl_->createValue(cmp == 0));
  }
  else if (cmd == "first") {
    if (numArgs != 3 && numArgs != 4) {
      tcl_->wrongNumArgs("should be \"string first subString string ?startIndex?\"");
      return CTclValueRef();
    }

    const std::string &substr = args[1]->toString();
    const std::string &str    = args[2]->toString();

    std::string::size_type pos = str.find(substr);

    return CTclValueRef(tcl_->createValue(int(pos)));
  }
  else if (cmd == "index") {
    if (numArgs != 3 && numArgs != 4) {
      tcl_->wrongNumArgs("should be \"string index string charIndex\"");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    int pos;

    bool ok = args[2]->toInt(pos);

    if (! ok) return CTclValueRef();

    int len = str.size();

    if (pos < 0 || pos >= len)
      return CTclValueRef();

    return CTclValueRef(tcl_->createValue(str.substr(pos, 1)));
  }
  else if (cmd == "is") {
    return CTclValueRef();
  }
  else if (cmd == "last") {
    if (numArgs != 3 && numArgs != 4) {
      tcl_->wrongNumArgs("should be \"string last subString string ?startIndex?\"");
      return CTclValueRef();
    }

    const std::string &substr = args[1]->toString();
    const std::string &str    = args[2]->toString();

    std::string::size_type pos = str.rfind(substr);

    return CTclValueRef(tcl_->createValue(int(pos)));
  }
  else if (cmd == "length") {
    if (numArgs != 2) {
      tcl_->wrongNumArgs("string length string");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    int len = str.size();

    return CTclValueRef(tcl_->createValue(len));
  }
  else if (cmd == "map") {
    return CTclValueRef();
  }
  else if (cmd == "match") {
    if (numArgs < 3) {
      tcl_->wrongNumArgs("string match ?-nocase? pattern string");
      return CTclValueRef();
    }

    bool        nocase = false;
    std::string str1, str2;

    uint pos = 1;

    if (pos < numArgs && args[pos]->toString() == "-nocase") {
      ++pos;

      nocase = true;
    }

    if (pos < numArgs)
      str1 = args[pos++]->toString();

    if (pos < numArgs)
      str2 = args[pos++]->toString();

    CGlob glob(str1);

    if (nocase)
      glob.setCaseSensitive(false);

    bool cmp = glob.compare(str2);

    return CTclValueRef(tcl_->createValue(cmp));
  }
  else if (cmd == "range") {
    if (numArgs != 4) {
      tcl_->wrongNumArgs("string range string first last");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    int ind1, ind2;

    if (! args[2]->toIndex(tcl_, ind1))
      return CTclValueRef();

    if (! args[3]->toIndex(tcl_, ind2))
      return CTclValueRef();

    std::string str1 = str.substr(ind1, ind2 - ind1);

    return CTclValueRef(tcl_->createValue(str1));
  }
  else if (cmd == "repeat") {
    if (numArgs != 3) {
      tcl_->wrongNumArgs("string repeat string count");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    int num;

    if (! args[2]->checkInt(tcl_, num))
      return CTclValueRef();

    std::string str1;

    for (int i = 0; i < num; ++i)
      str1 += str;

    return CTclValueRef(tcl_->createValue(str1));
  }
  else if (cmd == "replace") {
    if (numArgs != 4 && numArgs != 5) {
      tcl_->wrongNumArgs("string replace string first last ?string?");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    int start, end;

    if (! args[2]->checkInt(tcl_, start) || ! args[3]->checkInt(tcl_, end))
      return CTclValueRef();

    std::string rep_str;

    if (numArgs == 5)
      rep_str = args[4]->toString();

    int len = str.size();

    std::string lstr = (start >= 0 && start < len ? str.substr(0, start) : "");
    std::string rstr = (end   >= 0 && end   < len ? str.substr(end + 1): "");

    std::string str1 = lstr + rep_str + rstr;

    return CTclValueRef(tcl_->createValue(str1));
  }
  else if (cmd == "tolower") {
    if (numArgs < 2 || numArgs > 4) {
      tcl_->wrongNumArgs("string tolower string ?first? ?last?");
      return CTclValueRef();
    }

    std::string str = args[1]->toString();

    int len = str.size();

    int start = 0, end = len;

    if (numArgs > 2 && ! args[2]->checkInt(tcl_, start))
      return CTclValueRef();

    if (numArgs > 3 && ! args[3]->checkInt(tcl_, end))
      return CTclValueRef();

    for (int i = start; i <= end && i < len; ++i)
      str[i] = tolower(str[i]);

    return CTclValueRef(tcl_->createValue(str));
  }
  else if (cmd == "toupper") {
    if (numArgs < 2 || numArgs > 4) {
      tcl_->wrongNumArgs("string tolower string ?first? ?last?");
      return CTclValueRef();
    }

    std::string str = args[1]->toString();

    int len = str.size();

    int start = 0, end = len;

    if (numArgs > 2 && ! args[2]->checkInt(tcl_, start))
      return CTclValueRef();

    if (numArgs > 3 && ! args[3]->checkInt(tcl_, end))
      return CTclValueRef();

    for (int i = start; i <= end && i < len; ++i)
      str[i] = toupper(str[i]);

    return CTclValueRef(tcl_->createValue(str));
  }
  else if (cmd == "totitle") {
    if (numArgs < 2 || numArgs > 4) {
      tcl_->wrongNumArgs("string tolower string ?first? ?last?");
      return CTclValueRef();
    }

    std::string str = args[1]->toString();

    int len = str.size();

    int start = 0, end = len;

    if (numArgs > 2 && ! args[2]->checkInt(tcl_, start))
      return CTclValueRef();

    if (numArgs > 3 && ! args[3]->checkInt(tcl_, end))
      return CTclValueRef();

    for (int i = start; i <= end && i < len; ++i) {
      if (i == start)
        str[i] = toupper(str[i]);
      else
        str[i] = tolower(str[i]);
    }

    return CTclValueRef(tcl_->createValue(str));
  }
  else if (cmd == "trim") {
    if (numArgs < 2 || numArgs > 3) {
      tcl_->wrongNumArgs("string trim string ?chars?");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    uint len = str.size();

    uint l = 0;
    uint r = len - 1;

    if (numArgs == 2) {
      while (l <= r) {
        if (! isspace(str[l])) break;

        ++l;
      }

      while (r >= l) {
        if (! isspace(str[r])) break;

        --r;
      }
    }
    else {
      std::string chars = args[2]->toString();

      while (l <= r) {
        if (chars.find(str[l]) == std::string::npos) break;;

        ++l;
      }

      while (r >= l) {
        if (chars.find(str[r]) == std::string::npos) break;;

        --r;
      }
    }

    std::string str1 = str.substr(l, r - l + 1);

    return CTclValueRef(tcl_->createValue(str1));
  }
  else if (cmd == "trimleft") {
    if (numArgs < 2 || numArgs > 3) {
      tcl_->wrongNumArgs("string trimleft string ?chars?");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    uint len = str.size();

    uint l = 0;
    uint r = len - 1;

    if (numArgs == 2) {
      while (l <= r) {
        if (! isspace(str[l])) break;

        ++l;
      }
    }
    else {
      std::string chars = args[2]->toString();

      while (l <= r) {
        if (chars.find(str[l]) == std::string::npos) break;;

        ++l;
      }
    }

    std::string str1 = str.substr(l, r - l + 1);

    return CTclValueRef(tcl_->createValue(str1));
  }
  else if (cmd == "trimright") {
    if (numArgs < 2 || numArgs > 3) {
      tcl_->wrongNumArgs("string trimright string ?chars?");
      return CTclValueRef();
    }

    const std::string &str = args[1]->toString();

    uint len = str.size();

    uint l = 0;
    uint r = len - 1;

    if (numArgs == 2) {
      while (r >= l) {
        if (! isspace(str[r])) break;

        --r;
      }
    }
    else {
      std::string chars = args[2]->toString();

      while (r >= l) {
        if (chars.find(str[r]) == std::string::npos) break;;

        --r;
      }
    }

    std::string str1 = str.substr(l, r - l + 1);

    return CTclValueRef(tcl_->createValue(str1));
  }
  else if (cmd == "wordend") {
    return CTclValueRef();
  }
  else if (cmd == "wordstart") {
    return CTclValueRef();
  }
  else {
    tcl_->throwError("bad option \"" + cmd + "\": must be "
                     "bytelength, compare, equal, first, index, is, last, length, map, "
                     "match, range, repeat, replace, tolower, toupper, totitle, trim, "
                     "trimleft, trimright, wordend, or wordstart");
    return CTclValueRef();
  }
}

//----------

CTclValueRef
CTclSwitchCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  bool is_exact  = true;
  bool is_glob   = false;
  bool is_regexp = false;

  uint pos = 0;

  while (pos < numArgs) {
    const std::string &arg = args[pos]->toString();

    if (arg.size() > 0 && arg[0] == '-') {
      ++pos;

      if      (arg == "--"     ) break;
      else if (arg == "-exact" ) is_exact  = true;
      else if (arg == "-glob"  ) is_glob   = true;
      else if (arg == "-regexp") is_regexp = true;
    }
    else
      break;
  }

  if (pos >= numArgs - 1) {
    tcl_->wrongNumArgs("switch ?switches? string pattern body ... ?default body?");
    return CTclValueRef();
  }

  const std::string &str = args[pos++]->toString();

  bool extra_pattern = false;

  std::vector<std::string>  patterns;
  std::vector<CTclValueRef> bodies;

  if (pos == numArgs - 1) {
    // single list of pattern body
    CTclValueRef list;

    if (args[pos]->getType() == CTclValue::LIST_TYPE)
      list = args[pos];
    else
      list = args[pos]->toList(tcl_);

    uint length = list->getLength();

    uint i = 0;

    for ( ; i < length - 1; i += 2) {
      const std::string &pattern = list->getIndexValue(i)->toString();
      CTclValueRef       body    = list->getIndexValue(i + 1);

      patterns.push_back(pattern);
      bodies  .push_back(body);
    }

    if (i < length)
      extra_pattern = true;
  }
  else {
    while (pos < numArgs - 1) {
      const std::string &pattern = args[pos]->toString();
      CTclValueRef       body    = args[pos + 1];

      patterns.push_back(pattern);
      bodies  .push_back(body);

      pos += 2;
    }

    if (pos < numArgs)
      extra_pattern = true;
  }

  if (extra_pattern) {
    tcl_->throwError("extra switch pattern with no body");
    return CTclValueRef();
  }

  uint num_patterns = patterns.size();

  if (is_regexp) {
    CRegExp regexp(str);

    for (uint i = 0; i < num_patterns; ++i) {
      const std::string &pattern = patterns[i];
      CTclValueRef       body    = bodies  [i];

      if (i == num_patterns - 1 && pattern == "default") {
        body->exec(tcl_);
        break;
      }

      if (regexp.find(pattern)) {
        body->exec(tcl_);
        break;
      }
    }
  }
  else if (is_glob) {
    CGlob glob(str);

    for (uint i = 0; i < num_patterns; ++i) {
      const std::string &pattern = patterns[i];
      CTclValueRef       body    = bodies  [i];

      if (i == num_patterns - 1 && pattern == "default") {
        body->exec(tcl_);
        break;
      }

      if (glob.compare(pattern)) {
        body->exec(tcl_);
        break;
      }
    }
  }
  else {
    for (uint i = 0; i < num_patterns; ++i) {
      const std::string &pattern = patterns[i];
      CTclValueRef       body    = bodies  [i];

      if (i == num_patterns - 1 && pattern == "default") {
        body->exec(tcl_);
        break;
      }

      if (str == pattern) {
        body->exec(tcl_);
        break;
      }
    }
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclUpdateCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs > 1) {
    tcl_->wrongNumArgs("update ?idletasks?");
    return CTclValueRef();
  }

  bool idletasks = false;

  if (numArgs == 1) {
    const std::string &arg = args[0]->toString();

    if (arg == "idletasks")
      idletasks = true;
    else {
      tcl_->throwError("bad option \"" + arg + "\": must be idletasks");
      return CTclValueRef();
    }
  }

  tcl_->update(idletasks);

  return CTclValueRef();
}

//----------

CTclValueRef
CTclUnsetCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  for (uint i = 0; i < numArgs; ++i) {
    const std::string &varName = args[i]->toString();

    tcl_->removeVariable(varName);
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclVariableCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs < 1) {
    tcl_->wrongNumArgs("variable ?name value...? name ?value?");
    return CTclValueRef();
  }

  CTclScope *scope = tcl_->getScope();

  if (numArgs == 1) {
    const std::string &name = args[0]->toString();

    CTclVariableRef var = scope->getVariable(name);

    if (! var.isValid())
      scope->addVariable(name, CTclValueRef());
  }
  else {
    for (uint i = 0; i < numArgs - 1; i += 2) {
      const std::string &name  = args[i    ]->toString();
      CTclValueRef       value = args[i + 1];

      CTclVariableRef var = scope->getVariable(name);

      if (var.isValid())
        var->setValue(value);
      else
        var = scope->addVariable(name, value);

      scope->addVariable(name, var);
    }
  }

  return CTclValueRef();
}

//----------

CTclValueRef
CTclWhileCommand::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  if (numArgs != 2) {
    tcl_->wrongNumArgs("while test command");
    return CTclValueRef();
  }

  tcl_->setBreakFlag(false);

  while (args[0]->evalBool(tcl_)) {
    tcl_->setContinueFlag(false);

    args[1]->exec(tcl_);

    if (tcl_->getBreakFlag() || tcl_->getReturnFlag()) break;
  }

  tcl_->setBreakFlag   (false);
  tcl_->setContinueFlag(false);

  return CTclValueRef();
}

//-----------

CTclValueRef
CTclProc::
exec(const std::vector<CTclValueRef> &args)
{
  uint numArgs = args.size();

  uint numProcArgs = args_.size();

  bool var_args = (numProcArgs > 0 && args_[numProcArgs - 1] == "args");

  if (! var_args) {
    if (numArgs != numProcArgs) {
      std::string usage = name_;

      for (uint i = 0; i < numProcArgs; ++i)
        usage += " " + args_[i];

      tcl_->throwError("wrong # args: should be \"" + usage + "\"");

      return CTclValueRef();
    }
  }
  else {
    if (numArgs < numProcArgs - 1) {
      std::string usage = name_;

      for (uint i = 0; i < numProcArgs; ++i)
        usage += " " + args_[i];

      tcl_->throwError("wrong # args: should be \"" + usage + "\"");

      return CTclValueRef();
    }
  }

  CTclScope *pscope = tcl_->getScope();

  CTclScope *scope = new CTclScope(tcl_, pscope);

  tcl_->pushScope(scope);

  if (! var_args) {
    for (uint i = 0; i < numProcArgs; ++i)
      scope->setVariableValue(args_[i], args[i]);
  }
  else {
    for (uint i = 0; i < numProcArgs - 1; ++i)
      scope->setVariableValue(args_[i], args[i]);

    CTclList *list = new CTclList;

    for (uint i = numProcArgs - 1; i < numArgs; ++i)
      list->addValue(args[i]);

    scope->setVariableValue("args", CTclValueRef(list));
  }

  const std::string &bodyStr = body_->toString();

  CTclValueRef value = tcl_->parseString(bodyStr);

  delete scope;

  tcl_->popScope();

  CTclValueRef retVal;

  if (tcl_->getReturnFlag(retVal))
    value = retVal;

  tcl_->setReturnFlag(false);

  return value;
}

//-----------

bool
operator<(CTclValueRef lhs, CTclValueRef rhs) {
  if (lhs->getType() != rhs->getType()) return (lhs->getType() < rhs->getType());

  return lhs->cmp(rhs);
}
