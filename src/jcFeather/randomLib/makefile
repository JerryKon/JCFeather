OUTPUT_DIR=./out
OUTPUT_FILE=$(OUTPUT_DIR)/libRandomLib.so

CPPFILES=$(wildcard *.cpp)
HEADFILES=$(patsubst %.cpp, %.h, $(CPPFILES))  
OBJFILES=$(patsubst %.cpp, %.o, $(CPPFILES))  

$(OUTPUT_FILE):$(OBJFILES)
	-@mkdir $(OUTPUT_DIR)
	g++ -shared -o $@ $^
	-cp $(OUTPUT_FILE) ../../../bin/
%.o:%.cpp
	g++ $(CXXFLAGS) -o $@ $<

.PHONY :clean
clean:
	-rm -R $(OUTPUT_DIR) *.o
