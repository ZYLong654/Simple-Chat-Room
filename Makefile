object:server client

all:$(object) server.c client.c server.h client.h

server:server.c archives.c
	gcc -o $@ $^ -pthread

client:client.c
	gcc -o $@ $< -pthread

clean:
	rm server client
