.PHONY: all clean

all: transmitter receiver

receiver: receiver.c
	gcc receiver.c -o receiver

transmitter: transmitter.c
	gcc transmitter.c -o transmitter

clean:
	rm -rf *.out transmitter receiver
