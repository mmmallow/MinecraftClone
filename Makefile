BIN = bin
EXE = MinecraftClone

all: $(BIN)
	cd $(BIN) && make

bin:
	mkdir -p $(BIN)
	cd $(BIN) && cmake ../

run: all
	./$(BIN)/${EXE}

clean:
	rm $(BIN)/${EXE}

wipe:
	rm -rf $(BIN)

db: all
	gdb ./$(BIN)/${EXE}
