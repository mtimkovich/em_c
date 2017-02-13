EXE=em
FILES=em.c linenoise.c
OUT=$(addprefix src/,$(FILES))

$(EXE): $(OUT)
	$(CC) -o $(EXE) $(OUT)

clean:
	rm $(EXE)
