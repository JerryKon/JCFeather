OUTPUT_DIR=./out
OUTPUT_FILE=$(OUTPUT_DIR)/libjcFeather.so

INCLUDES=-I$(BOOST_HOME)/include \
-I$(MAYA_LOCATION)/include \
-I$(ZLIB)/include \
-I../tinyXml \
-I../randomLib \
-I../featherGen 

LDPATH=-L../randomLib/out \
-L../tinyXml/out \
-L../featherGen/out \
-L$(BOOST_HOME)/lib \
-L$(MAYA_LOCATION)/lib \
-L$(ZLIB)/lib 

LDFLAGS=-lTinyXml -lRandomLib -lFeatherGen -lboost_serialization -lboost_filesystem \
-lz -lOpenMaya -lOpenMayaAnim -lFoundation -shared 

CPPFILES=$(wildcard *.cpp)
HEADFILES=$(patsubst %.cpp, %.H, $(CPPFILES))
OBJFILES=$(patsubst %.cpp, %.o, $(CPPFILES))  


$(OUTPUT_FILE):$(OBJFILES)
	-@mkdir $(OUTPUT_DIR)
	g++ -o $@ $^ $(LDPATH) $(LDFLAGS) 
	-cp $@ ../../../bin/
%.o:%.cpp
	g++ -o $@ $^ $(CXXFLAGS) $(MAYA_CXXFLAGS) $(INCLUDES)

.PHONY :clean
clean:
	-rm -R $(OUTPUT_DIR) *.o
