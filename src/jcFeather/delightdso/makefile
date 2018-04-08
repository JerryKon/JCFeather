OUTPUT_DIR=./out
OUTPUT_FILE=$(OUTPUT_DIR)/delightdso.so

INCLUDES=-I$(DELIGHT)/include \
-I../jcFeatherLib 

LDPATH=-L../jcFeatherLib/out -L$(DELIGHT)/lib

LDFLAGS=-ljcFeather -l3delight -shared 

CPPFILES=$(wildcard *.cpp)
HEADFILES=$(patsubst %.cpp, %.H, $(CPPFILES))
OBJFILES=$(patsubst %.cpp, %.o, $(CPPFILES))  


$(OUTPUT_FILE):$(OBJFILES)
	-@mkdir $(OUTPUT_DIR)
	g++ -o $@ $^ $(LDPATH) $(LDFLAGS) 
	-cp $@ ../../../bin/
%.o:%.cpp
	g++ -o $@ $^ $(CXXFLAGS) $(INCLUDES)

.PHONY :clean
clean:
	-rm -R $(OUTPUT_DIR) *.o
