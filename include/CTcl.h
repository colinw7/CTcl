#ifndef CTCL_H
#define CTCL_H

#include <CRefPtr.h>
#include <CBool.h>

#include <map>
#include <list>
#include <vector>
#include <string>
#include <iostream>
#include <sys/types.h>

class CTcl;
class CStrParse;
class CTclValue;
class CTclTimer;
class CHistory;

using CTclValueRef = CRefPtr<CTclValue>;

class CTclValue {
 public:
  enum class ValueType {
    NONE,
    STRING,
    ARRAY,
    LIST,
    VALUE_MAP
  };

 public:
  CTclValue(ValueType type) :
   type_(type) {
  }

  virtual ~CTclValue() { }

  ValueType getType() const { return type_; }

  virtual CTclValue *dup() const = 0;

  virtual int cmp(CTclValueRef rhs) const = 0;

  virtual void print(std::ostream &os) const = 0;

  virtual std::string toString() const = 0;

  virtual bool toInt (int    &) const { return false; }
  virtual bool toReal(double &) const { return false; }

  virtual bool toBool() const = 0;

  virtual CTclValueRef toList(CTcl *) const { return CTclValueRef(); }

  virtual uint getLength() const { return 1; }

  virtual CTclValueRef getIndexValue(uint i) const {
    assert(i == 0);

    return CTclValueRef(dup());
  }

  virtual void setIndexValue(uint i, CTclValueRef value) {
    assert(i == 0 && value.isValid());
  }

  virtual void addValue(CTclValueRef) { assert(false); }

  bool checkInt (CTcl *tcl, int    &i);
  bool checkReal(CTcl *tcl, double &r);

  bool toIndex(CTcl *tcl, int &ind);

  CTclValueRef eval(CTcl *tcl) const;

  CTclValueRef exec(CTcl *tcl) const;

  bool evalBool(CTcl *tcl) const;

 protected:
  CTclValue(const CTclValue &value);
  CTclValue &operator=(const CTclValue &value);

 protected:
  ValueType type_ { ValueType::NONE };
};

//---

bool operator<(CTclValueRef lhs, CTclValueRef rhs);

//---

class CTclString : public CTclValue {
 public:
  CTclString(const std::string &str="") :
   CTclValue(ValueType::STRING), str_(str) {
  }

 ~CTclString() { }

  CTclString *dup() const { return new CTclString(str_); }

  int cmp(CTclValueRef rhs) const {
    CTclString *str = rhs.cast<CTclString>();

    if      (str_ < str->str_) return -1;
    else if (str_ > str->str_) return  1;
    else                       return  0;
  }

  void print(std::ostream &os) const;

  std::string toString() const;

  bool toInt (int    &i) const;
  bool toReal(double &r) const;

  bool toBool() const;

  CTclValueRef toList(CTcl *tcl) const;

  const std::string &getValue() const { return str_; }

  void setValue(const std::string &str) { str_ = str; }

  void appendValue(const std::string &str) { str_ += str; }

 private:
  std::string str_;
};

//---

class CTclArray : public CTclValue {
 public:
  using ValueMap = std::map<std::string,CTclValueRef>;

 public:
  CTclArray() :
   CTclValue(ValueType::ARRAY) {
  }

  CTclArray(const ValueMap &values) :
   CTclValue(ValueType::ARRAY), values_(values) {
  }

 ~CTclArray() { }

  CTclArray *dup() const { return new CTclArray(values_); }

  int cmp(CTclValueRef rhs) const {
    CTclArray *array = rhs.cast<CTclArray>();

    uint numValues1 = uint(       values_.size());
    uint numValues2 = uint(array->values_.size());

    if      (numValues1 < numValues2) return -1;
    else if (numValues1 > numValues2) return  1;
    else                                return  0;

    ValueMap::const_iterator p1, p2, pe;

    for (p1 = values_.begin(), p2 = array->values_.begin(), pe =values_.end();
          p1 != pe; ++p1, ++p2) {
       const std::string &key1 = (*p1).first;
       const std::string &key2 = (*p2).first;

       if      (key1 < key2) return -1;
       else if (key1 > key2) return  1;

       CTclValueRef value1 = (*p1).second;
       CTclValueRef value2 = (*p2).second;

       int val = value1->cmp(value2);

       if (val != 0) return val;
    }

    return 0;
  }

  void getNames(std::vector<std::string> &names) {
    ValueMap::const_iterator p1, p2;

    for (p1 = values_.begin(), p2 = values_.end(); p1 != p2; ++p1)
      names.push_back((*p1).first);
  }

  void getNameValues(std::vector<std::string> &names, std::vector<CTclValueRef> &values) {
    ValueMap::const_iterator p1, p2;

    for (p1 = values_.begin(), p2 = values_.end(); p1 != p2; ++p1) {
      names .push_back((*p1).first );
      values.push_back((*p1).second);
    }
  }

  void print(std::ostream &os) const;

  bool toBool() const;

  std::string toString() const;

  CTclValueRef getValue(const std::string &indexStr) {
    ValueMap::const_iterator p = values_.find(indexStr);

    if (p == values_.end())
      return CTclValueRef();

    return (*p).second;
  }

  void setValue(const std::string &indexStr, CTclValueRef value) {
    values_[indexStr] = value->dup();
  }

 private:
  ValueMap values_;
};

//---

class CTclList : public CTclValue {
 public:
  using ValueList = std::vector<CTclValueRef>;

 public:
  CTclList(const ValueList &values=ValueList()) :
   CTclValue(ValueType::LIST), values_(values) {
  }

 ~CTclList() { }

  CTclList *dup() const { return new CTclList(values_); }

  int cmp(CTclValueRef rhs) const {
    CTclList *list = rhs.cast<CTclList>();

    uint numValues1 = uint(      values_.size());
    uint numValues2 = uint(list->values_.size());

    if      (numValues1 < numValues2) return -1;
    else if (numValues1 > numValues2) return  1;
    else                              return  0;

    for (uint i = 0; i < numValues1; ++i) {
      int val = values_[i]->cmp(list->values_[i]);

      if (val != 0) return val;
    }

    return 0;
  }

  bool toBool() const;

  std::string toString() const;

  CTclValueRef toList(CTcl *) const { return CTclValueRef(dup()); }

  uint getLength() const { return uint(values_.size()); }

  CTclValueRef getIndexValue(uint i) const {
    assert(i < values_.size());

    return CTclValueRef(values_[i]->dup());
  }

  void setIndexValue(uint i, CTclValueRef value) {
    values_[i] = value;
  }

  void addValue(CTclValueRef value) {
    values_.push_back(CTclValueRef(value->dup()));
  }

  void print(std::ostream &os) const;

 private:
  ValueList values_;
};

//---

class CTclCommand {
 public:
  enum class CommandType {
    NONE      = 0,
    ITERATION = (1<<0)
  };

 public:
  CTclCommand(CTcl *tcl, const std::string &name) :
   tcl_(tcl), name_(name) {
  }

  virtual ~CTclCommand() { }

  const std::string &getName() const { return name_; }

  uint getType() const { return 0; }

  virtual CTclValueRef exec(const std::vector<CTclValueRef> &args) = 0;

 protected:
  CTcl*       tcl_ { nullptr };
  std::string name_;
};

//---

class CTclProc {
 public:
  using ArgList = std::vector<std::string>;

 public:
  CTclProc(CTcl *tcl, const std::string name, const ArgList &args, CTclValueRef body) :
   tcl_(tcl), name_(name), args_(args), body_(body) {
  }

  const std::string &getName() const { return name_; }

  void getArgs(ArgList &args) const { args = args_; }

  CTclValueRef getBody() const { return body_; }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);

 private:
  CTcl*        tcl_ { nullptr };
  std::string  name_;
  ArgList      args_;
  CTclValueRef body_;
};

//---

class CTclCommentCommand : public CTclCommand {
 public:
  CTclCommentCommand(CTcl *tcl) : CTclCommand(tcl, "#") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclAfterCommand : public CTclCommand {
 public:
  CTclAfterCommand(CTcl *tcl) : CTclCommand(tcl, "after") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclAppendCommand : public CTclCommand {
 public:
  CTclAppendCommand(CTcl *tcl) : CTclCommand(tcl, "append") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclArrayCommand : public CTclCommand {
 public:
  CTclArrayCommand(CTcl *tcl) : CTclCommand(tcl, "array") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclBreakCommand : public CTclCommand {
 public:
  CTclBreakCommand(CTcl *tcl) : CTclCommand(tcl, "break") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclCatchCommand : public CTclCommand {
 public:
  CTclCatchCommand(CTcl *tcl) : CTclCommand(tcl, "catch") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclCDCommand : public CTclCommand {
 public:
  CTclCDCommand(CTcl *tcl) : CTclCommand(tcl, "cd") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclClockCommand : public CTclCommand {
 public:
  CTclClockCommand(CTcl *tcl) : CTclCommand(tcl, "clock") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclCloseCommand : public CTclCommand {
 public:
  CTclCloseCommand(CTcl *tcl) : CTclCommand(tcl, "close") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclContinueCommand : public CTclCommand {
 public:
  CTclContinueCommand(CTcl *tcl) : CTclCommand(tcl, "continue") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclEchoCommand : public CTclCommand {
 public:
  CTclEchoCommand(CTcl *tcl) : CTclCommand(tcl, "echo") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclEofCommand : public CTclCommand {
 public:
  CTclEofCommand(CTcl *tcl) : CTclCommand(tcl, "eof") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclEvalCommand : public CTclCommand {
 public:
  CTclEvalCommand(CTcl *tcl) : CTclCommand(tcl, "eval") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclExecCommand : public CTclCommand {
 public:
  CTclExecCommand(CTcl *tcl) : CTclCommand(tcl, "exec") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclExitCommand : public CTclCommand {
 public:
  CTclExitCommand(CTcl *tcl) : CTclCommand(tcl, "exit") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclExprCommand : public CTclCommand {
 public:
  CTclExprCommand(CTcl *tcl) : CTclCommand(tcl, "expr") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclFileCommand : public CTclCommand {
 public:
  CTclFileCommand(CTcl *tcl) : CTclCommand(tcl, "file") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclFlushCommand : public CTclCommand {
 public:
  CTclFlushCommand(CTcl *tcl) : CTclCommand(tcl, "flush") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclForCommand : public CTclCommand {
 public:
  CTclForCommand(CTcl *tcl) : CTclCommand(tcl, "for") { }

  uint getType() const { return uint(CommandType::ITERATION); }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclForeachCommand : public CTclCommand {
 public:
  CTclForeachCommand(CTcl *tcl) : CTclCommand(tcl, "foreach") { }

  uint getType() const { return uint(CommandType::ITERATION); }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclFormatCommand : public CTclCommand {
 public:
  CTclFormatCommand(CTcl *tcl) : CTclCommand(tcl, "format") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclGetsCommand : public CTclCommand {
 public:
  CTclGetsCommand(CTcl *tcl) : CTclCommand(tcl, "gets") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclGlobCommand : public CTclCommand {
 public:
  CTclGlobCommand(CTcl *tcl) : CTclCommand(tcl, "glob") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclGlobalCommand : public CTclCommand {
 public:
  CTclGlobalCommand(CTcl *tcl) : CTclCommand(tcl, "global") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclHistoryCommand : public CTclCommand {
 public:
  CTclHistoryCommand(CTcl *tcl) : CTclCommand(tcl, "history") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclIfCommand : public CTclCommand {
 public:
  CTclIfCommand(CTcl *tcl) : CTclCommand(tcl, "if") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclIncrCommand : public CTclCommand {
 public:
  CTclIncrCommand(CTcl *tcl) : CTclCommand(tcl, "incr") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclInfoCommand : public CTclCommand {
 public:
  CTclInfoCommand(CTcl *tcl) : CTclCommand(tcl, "info") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclJoinCommand : public CTclCommand {
 public:
  CTclJoinCommand(CTcl *tcl) : CTclCommand(tcl, "join") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLAppendCommand : public CTclCommand {
 public:
  CTclLAppendCommand(CTcl *tcl) : CTclCommand(tcl, "lappend") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclListCommand : public CTclCommand {
 public:
  CTclListCommand(CTcl *tcl) : CTclCommand(tcl, "list") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLIndexCommand : public CTclCommand {
 public:
  CTclLIndexCommand(CTcl *tcl) : CTclCommand(tcl, "lindex") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLInsertCommand : public CTclCommand {
 public:
  CTclLInsertCommand(CTcl *tcl) : CTclCommand(tcl, "linsert") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLLengthCommand : public CTclCommand {
 public:
  CTclLLengthCommand(CTcl *tcl) : CTclCommand(tcl, "llength") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLRangeCommand : public CTclCommand {
 public:
  CTclLRangeCommand(CTcl *tcl) : CTclCommand(tcl, "lrange") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLRepeatCommand : public CTclCommand {
 public:
  CTclLRepeatCommand(CTcl *tcl) : CTclCommand(tcl, "lrepeat") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLReplaceCommand : public CTclCommand {
 public:
  CTclLReplaceCommand(CTcl *tcl) : CTclCommand(tcl, "lreplace") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLSearchCommand : public CTclCommand {
 public:
  CTclLSearchCommand(CTcl *tcl) : CTclCommand(tcl, "lsearch") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLSetCommand : public CTclCommand {
 public:
  CTclLSetCommand(CTcl *tcl) : CTclCommand(tcl, "lset") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclLSortCommand : public CTclCommand {
 public:
  CTclLSortCommand(CTcl *tcl) : CTclCommand(tcl, "lsort") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclNamespaceCommand : public CTclCommand {
 public:
  CTclNamespaceCommand(CTcl *tcl) : CTclCommand(tcl, "namespace") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclOpenCommand : public CTclCommand {
 public:
  CTclOpenCommand(CTcl *tcl) : CTclCommand(tcl, "open") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclProcCommand : public CTclCommand {
 public:
  CTclProcCommand(CTcl *tcl) : CTclCommand(tcl, "proc") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclPutsCommand : public CTclCommand {
 public:
  CTclPutsCommand(CTcl *tcl) : CTclCommand(tcl, "puts") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclPidCommand : public CTclCommand {
 public:
  CTclPidCommand(CTcl *tcl) : CTclCommand(tcl, "pid") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclPwdCommand : public CTclCommand {
 public:
  CTclPwdCommand(CTcl *tcl) : CTclCommand(tcl, "pwd") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclReadCommand : public CTclCommand {
 public:
  CTclReadCommand(CTcl *tcl) : CTclCommand(tcl, "read") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclReturnCommand : public CTclCommand {
 public:
  CTclReturnCommand(CTcl *tcl) : CTclCommand(tcl, "return") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclSetCommand : public CTclCommand {
 public:
  CTclSetCommand(CTcl *tcl) : CTclCommand(tcl, "set") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclSourceCommand : public CTclCommand {
 public:
  CTclSourceCommand(CTcl *tcl) : CTclCommand(tcl, "source") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclStringCommand : public CTclCommand {
 public:
  CTclStringCommand(CTcl *tcl) : CTclCommand(tcl, "string") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclSwitchCommand : public CTclCommand {
 public:
  CTclSwitchCommand(CTcl *tcl) : CTclCommand(tcl, "switch") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclUpdateCommand : public CTclCommand {
 public:
  CTclUpdateCommand(CTcl *tcl) : CTclCommand(tcl, "update") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclUnsetCommand : public CTclCommand {
 public:
  CTclUnsetCommand(CTcl *tcl) : CTclCommand(tcl, "unset") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclVariableCommand : public CTclCommand {
 public:
  CTclVariableCommand(CTcl *tcl) : CTclCommand(tcl, "variable") { }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

class CTclWhileCommand : public CTclCommand {
 public:
  CTclWhileCommand(CTcl *tcl) : CTclCommand(tcl, "while") { }

  uint getType() const { return uint(CommandType::ITERATION); }

  CTclValueRef exec(const std::vector<CTclValueRef> &args);
};

//---

class CTclVariable;

class CTclVariableProc {
 public:
  CTclVariableProc() { }

  virtual ~CTclVariableProc() { }

  void setId(uint id) { id_ = id; }

  virtual void notify(CTclVariable *var) = 0;

 protected:
  uint id_ { 0 };
};

//---

class CTclVariable {
 public:
  CTclVariable(CTclValueRef value=CTclValueRef());

  virtual ~CTclVariable();

  std::string toString() const;

  bool toBool() const;

  virtual bool hasValue() const;

  virtual CTclValueRef getValue() const;

  virtual void setValue(CTclValueRef value);

  virtual CTclValueRef getArrayValue(const std::string &indexStr) const;

  virtual void setArrayValue(const std::string &indexStr, CTclValueRef value);

  virtual void appendValue(CTclValueRef value);

  uint addNotifyProc(CTclVariableProc *proc);

 private:
  void callNotifyProcs();

 private:
  using VariableProcList = std::list<CTclVariableProc *>;

  static uint notifyProcId_;

  CTclValueRef     value_;
  VariableProcList notifyProcs_;
};

using CTclVariableRef = CRefPtr<CTclVariable>;

//---

class CTclEnvVariable : public CTclVariable {
 public:
  CTclEnvVariable();

  CTclValueRef getArrayValue(const std::string &indexStr) const;

};

//---

class CTclScope {
 public:
  CTclScope(CTcl *tcl, CTclScope *parent=NULL, const std::string &name="");

 ~CTclScope();

  CTclScope *parentScope() const { return parent_; }

  CTclVariableRef addVariable(const std::string &varName, CTclValueRef value);
  CTclVariableRef addVariable(const std::string &varName, CTclVariableRef var);

  CTclVariableRef getVariable(const std::string &varName);

  CTclValueRef getVariableValue(const std::string &varName);

  void setVariableValue(const std::string &varName, CTclValueRef value);

  CTclValueRef getArrayVariableValue(const std::string &varName, const std::string &indexStr);

  void setArrayVariableValue(const std::string &varName, const std::string &indexStr,
                             CTclValueRef value);

  void getVariableNames(std::vector<std::string> &names) const;

  void removeVariable(const std::string &varName);

  CTclProc *defineProc(const std::string &name, const std::vector<std::string> &args,
                       CTclValueRef body);

  CTclProc *getProc(const std::string &varName);

  void removeProc(const std::string &name);

  void getProcNames(std::vector<std::string> &names);

  CTclScope *getNamedScope(const std::string &name, bool create_it=false);

 private:
  using VariableList = std::map<std::string,CTclVariableRef>;
  using ProcList     = std::map<std::string,CTclProc *>;
  using ScopeMap     = std::map<std::string,CTclScope *>;

  CTcl*        tcl_    { nullptr };
  CTclScope*   parent_ { nullptr };
  std::string  name_;
  VariableList vars_;
  ProcList     procs_;
  ScopeMap     scopeMap_;
};

//---

class CTclError {
 public:
  CTclError(const std::string &msg) : msg_(msg) { }

  const std::string &getMsg() const { return msg_; }

 private:
  std::string msg_;
};

//---

class CTcl {
 private:
  class SetSeparator {
   public:
    SetSeparator(CTcl *tcl, char c) :
     tcl_(tcl) {
      old_c_ = tcl_->getSeparator();

      tcl_->setSeparator(c);
    }

   ~SetSeparator() {
      tcl_->setSeparator(old_c_);
    }

   private:
    CTcl* tcl_   { nullptr };
    char  old_c_ { '\0' };
  };

 public:
  CTcl(int argc, char **argv);
 ~CTcl();

  bool getBreakFlag() const { return breakFlag_; }
  void setBreakFlag(bool flag=true) { breakFlag_ = flag; }

  bool getContinueFlag() const { return continueFlag_; }
  void setContinueFlag(bool flag=true) { continueFlag_ = flag; }

  bool getReturnFlag() const { return returnFlag_; }
  bool getReturnFlag(CTclValueRef &val) const { val = returnVal_; return returnFlag_; }
  void setReturnFlag(bool flag=false) { returnFlag_ = flag; }
  void setReturnFlag(CTclValueRef val, bool flag=true) { returnVal_ = val; returnFlag_ = flag; }

  bool getDebug() const { return debug_; }
  void setDebug(bool debug=true) { debug_ = debug; }

  char getSeparator() const { return separator_; }
  void setSeparator(char c) { separator_ = c; }

  bool parseFile(const std::string &filename);

  bool isCompleteLine(const std::string &line);

  CTclValueRef parseLine(const std::string &str);

  CTclValueRef parseString(const std::string &str);

  bool processLine(const std::string &line);

  bool readArgList(std::vector<CTclValueRef> &args);

  bool readExecString(std::vector<CTclValueRef> &args);

  CTclList *stringToList(const std::string &str);

  bool readLiteralString(std::string &str);

  bool readDoubleQuotedString(std::string &str);

  bool readSingleQuotedString(std::string &str);

  bool readVariableName(std::string &varName, std::string &indexName, bool &is_array);

  bool readWord(std::string &str, char endChar);

  std::string expandExpr(const std::string &str);

  void addHistory(const std::string &str);

  CTclScope *getNamedScope(const std::string &name, bool create_it=false);

  void pushScope(CTclScope *scope);
  void popScope();
  void unwindScope();

  void         startCommand(CTclCommand *cmd);
  void         endCommand();
  CTclCommand *getCommand() const;

  void      startProc(CTclProc *proc);
  void      endProc();
  CTclProc *getProc() const;

  void addCommand(CTclCommand *command);

  CTclCommand *getCommand(const std::string &name);

  void getCommandNames(std::vector<std::string> &names) const;

  CTclVariableRef addVariable(const std::string &varName, CTclValueRef value);
  CTclVariableRef addVariable(const std::string &varName, CTclVariableRef var);

  CTclVariableRef getVariable(const std::string &varName);

  CTclValueRef getVariableValue(const std::string &varName);

  void setVariableValue(const std::string &varName, CTclValueRef value);

  CTclValueRef getArrayVariableValue(const std::string &varName, const std::string &indexStr);

  void setArrayVariableValue(const std::string &varName, const std::string &indexStr,
                             CTclValueRef value);

  void removeVariable(const std::string &varName);

  CTclProc *defineProc(const std::string &name, const std::vector<std::string> &args,
                       CTclValueRef body);

  CTclProc *getProc(const std::string &varName);

  CTclScope *getScope() const { return scope_; }

  CTclScope *getGlobalScope() const { return gscope_; }

  CHistory *getHistory() const { return history_; }

  std::string openFile(const std::string &fileName, const std::string &mode);
  void closeFile(const std::string &handle);
  void writeToFile(const std::string &handle, const std::string &str);
  FILE *getFile(const std::string &handle);

  std::string addTimer   (int ms, const std::string &script);
  bool        cancelTimer(const std::string &id);

  void insertTimer(CTclTimer *timer);
  void clearTimer (CTclTimer *timer);

  void       getTimerNames(std::vector<std::string> &names);
  CTclTimer *getTimer(const std::string &id);

  void update(bool idletasks);

  CTclValueRef createValue(int value) const;
  CTclValueRef createValue(uint value) const;
  CTclValueRef createValue(double value) const;
  CTclValueRef createValue(const std::string &value) const;
  CTclValueRef createValue(const std::vector<std::string> &strs) const;
  CTclValueRef createValue(const std::vector<CTclValueRef> &values) const;

  CTclValueRef evalString(const std::string &str);

  CTclValueRef evalArgs(const std::vector<CTclValueRef> &args);

  std::string lookupPathCommand(const std::string &name) const;

  void startFileParse(const std::string &fileName);
  void startStringParse(const std::string &str);
  void endParse();

  void wrongNumArgs(const std::string &msg);

  void badInteger(CTclValueRef value);
  void badReal(CTclValueRef value);

  void throwError(const std::string &msg);

  static bool needsBraces(const std::string &str);

 private:
  bool isCompleteLine1(char endChar);

 private:
  using CommandStack = std::vector<CTclCommand *>;
  using ProcStack    = std::vector<CTclProc *>;
  using CommandList  = std::map<std::string,CTclCommand *>;
  using ScopeStack   = std::vector<CTclScope *>;
  using ParseStack   = std::vector<CStrParse *>;
  using FileMap      = std::map<std::string,FILE *>;
  using TimerMap     = std::map<std::string,CTclTimer *>;

  CStrParse*   parse_     { nullptr };
  ParseStack   parseStack_;
  CommandList  cmds_;
  ScopeStack   scopeStack_;
  CTclScope*   scope_     { nullptr };
  CTclScope*   gscope_    { nullptr };
  CommandStack cmdStack_;
  ProcStack    procStack_;
  CHistory*    history_   { nullptr };
  FileMap      fileMap_;
  TimerMap     timerMap_;
  char         separator_ { ';' };
  CBool        breakFlag_;
  CBool        continueFlag_;
  CBool        returnFlag_;
  CTclValueRef returnVal_;
  bool         debug_     { false };
};

#endif
