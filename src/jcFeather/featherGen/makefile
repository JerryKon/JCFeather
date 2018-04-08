OUTPUT_DIR=./out
OUTPUT_FILE=$(OUTPUT_DIR)/libFeatherGen.a

INCLUDES=-I$(BOOST_HOME)/include \
-I$(MAYA_LOCATION)/include \
-I$(ZLIB)/include \
-I../tinyXml \
-I../randomLib


CPPFILES=$(wildcard *.cpp)
HEADFILES=$(patsubst %.cpp, %.H, $(CPPFILES))
OBJFILES=$(patsubst %.cpp, %.o, $(CPPFILES))  


$(OUTPUT_FILE):$(OBJFILES)
	-@mkdir $(OUTPUT_DIR)
	ar -r $@ $^

%.o:%.cpp
	g++ $(CXXFLAGS) $(MAYA_CXXFLAGS) $(INCLUDES) -c -o $@ $<

.PHONY :clean
clean:
	-rm -R $(OUTPUT_DIR) *.o
