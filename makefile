TARGETS = libnetfiles client

CC = gcc
FLAGS = -Wall

all: clean $(TARGETS)

$(TARGETS):
	$(CC) $(FLAGS) $@.c -o client 

clean:
	rm -f $(TARGETS)
