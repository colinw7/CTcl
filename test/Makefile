CC = g++
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = .
LIB_DIR = ../lib
BIN_DIR = ../bin

all: dirs $(BIN_DIR)/CTclTest

dirs:
	@if [ ! -e ../bin ]; then mkdir ../bin; fi

SRC = \
CTclTest.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
$(CDEBUG) \
-std=c++14 \
-I. \
-I$(INC_DIR) \
-I../../CArgs/include \
-I../../CReadLine/include \
-I../../CUtil/include \
-I../../CRegExp/include \

LFLAGS = \
$(LDEBUG) \
-L$(LIB_DIR) \
-L../../CTcl/lib \
-L../../CCommand/lib \
-L../../CArgs/lib \
-L../../CReadLine/lib \
-L../../CTimer/lib \
-L../../CFile/lib \
-L../../CGlob/lib \
-L../../CUtil/lib \
-L../../CStrUtil/lib \
-L../../CRegExp/lib \
-L../../COS/lib \

LIBS = \
-lCTcl -lCCommand -lCArgs -lCTimer -lCReadLine -lCFile -lCUtil \
-lCStrUtil -lCGlob -lCRegExp -lCOS \
-lreadline -ltre -lncurses

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(BIN_DIR)/CTclTest

.SUFFIXES: .cpp

.cpp.o:
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(BIN_DIR)/CTclTest: $(OBJS) $(LIB_DIR)/libCTcl.a
	$(CC) $(LDEBUG) -o $(BIN_DIR)/CTclTest $(OBJS) $(LFLAGS) $(LIBS)
