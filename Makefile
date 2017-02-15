EXE=em
FILES=em.c linenoise.c
OUT=$(addprefix src/,$(FILES))

$(EXE): $(OUT)
	$(CC) -o $(EXE) $(OUT)

debug: $(OUT)
	$(CC) -o $(EXE) -g $(OUT)

clean:
	rm $(EXE)
