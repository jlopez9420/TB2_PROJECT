TARGET=DBCLI
C_SRCFILES=main.cpp cli.cpp databasemanager.cpp dbcli_lexer.cpp utilities.cpp
OBJ_FILES=${C_SRCFILES:.cpp=.o}
.PHONY: clean
.SUFFIXES:

$(TARGET): $(OBJ_FILES)
	g++ -g -o $@ $(OBJ_FILES)

%.o: %.cpp
	g++ -std=c++11 -g -c -o $@ $<

run: $(TARGET)
	./$(TARGET) Arith.pas

clean:
	rm -f *.o
	rm -f $(TARGET)