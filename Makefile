BIN = bin
EXE = MinecraftClone

all: $(BIN)
	cd $(BIN) && make

bin:
	mkdir -p $(BIN)
	mkdir -p data
	cd $(BIN) && cmake ../

run: all
	./$(BIN)/${EXE}

clean:
	rm $(BIN)/${EXE}
	rm data/*

wipe:
	rm -rf $(BIN)
	rm -rf data

db: all
	gdb ./$(BIN)/${EXE}
