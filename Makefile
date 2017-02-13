em: em.c linenoise.c
	$(CC) -o em em.c linenoise.c

clean:
	rm em
