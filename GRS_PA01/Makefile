CC = gcc
LIBS = -pthread -lm
CFLAGS = -Wall -g

PART_A_SRC_1 = MT25074_PART_A_Program_A.c
PART_A_SRC_2 = MT25074_PART_A_Program_B.c
SRC_WORKERS = MT25074_Part_B_Program.c

EXEC_A1 = program_a1
EXEC_A2 = program_a2


all: $(EXEC_A1) $(EXEC_A2)

$(EXEC_A1): $(PART_A_SRC_1) $(SRC_WORKERS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)
  
$(EXEC_A2): $(PART_A_SRC_2) $(SRC_WORKERS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o $(EXEC_A1) $(EXEC_A2)
