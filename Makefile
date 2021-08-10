# Makefile for the smash program
CC = g++
CFLAGS = -g -Wall 
CCLINK = $(CC)
OBJS = smash.o commands.o signals.o
RM = rm -f
# Creating the  executable
smash: $(OBJS)
	$(CCLINK) -o smash $(OBJS)
# Creating the object files
commands.o: commands.cc commands.h
smash.o: smash.cc commands.h
signals.o: signals.cc signals.h
# Cleaning old files before new make
clean:
	$(RM) $(TARGET) *.o *~ "#"* core.*

