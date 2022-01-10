CC = g++
AR = ar
RM = rm

CDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib
BIN_DIR = ../bin

all: dirs $(LIB_DIR)/libCTcl.a

dirs:
	@if [ ! -e ../obj ]; then mkdir ../obj; fi
	@if [ ! -e ../lib ]; then mkdir ../lib; fi
	@if [ ! -e ../bin ]; then mkdir ../bin; fi

SRC = \
CTcl.cpp \
CEval.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
-std=c++14 \
$(CDEBUG) \
-I. \
-I$(INC_DIR) \
-I../../CCommand/include \
-I../../CReadLine/include \
-I../../CFile/include \
-I../../CRegExp/include \
-I../../CGlob/include \
-I../../CTimer/include \
-I../../CStrUtil/include \
-I../../CUtil/include \
-I../../COS/include \

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(LIB_DIR)/libCTcl.a

.SUFFIXES: .cpp

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

$(LIB_DIR)/libCTcl.a: $(OBJS)
	$(AR) crv $(LIB_DIR)/libCTcl.a $(OBJS)
