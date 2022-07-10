
EXECUTABLE=break
SRC=src/break.c
CFLAGS=-g


break: $(SRC)
	$(CC) -o $(EXECUTABLE) $(SRC) $(CFLAGS)
