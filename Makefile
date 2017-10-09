cc = g++
cflags = -std=c++14 -Wall -O2
server_exec=siktacka-server
client_exec=siktacka-client

all: server client

server: additional_exceptions.o events.o helper.o messeges.o server_state.o udpmessenger.o server_main.o server.o
	$(cc) -o $(server_exec) $(cflags) additional_exceptions.o events.o helper.o messeges.o server_state.o udpmessenger.o server_main.o server.o -lz

client: additional_exceptions.o client.o client_state.o events.o helper.o messeges.o udpmessenger.o tcpmessenger.o client_main.o
	$(cc) -o $(client_exec) $(cflags) additional_exceptions.o client.o client_state.o events.o helper.o messeges.o udpmessenger.o tcpmessenger.o client_main.o -lz

additional_exceptions.o: additional_exceptions.cpp additional_exceptions.h
	$(cc) -c $(cflags) additional_exceptions.cpp

client_state.o: client_state.cpp client_state.h
	$(cc) -c $(cflags) client_state.cpp

client.o: client.cpp client.h
	$(cc) -c $(cflags) client.cpp

events.o: events.cpp events.h
	$(cc) -c $(cflags) events.cpp

helper.o: helper.cpp helper.h
	$(cc) -c $(cflags) helper.cpp

messeges.o: messeges.cpp messeges.h
	$(cc) -c $(cflags) messeges.cpp

server_state.o: server_state.cpp server_state.h
	$(cc) -c $(cflags) server_state.cpp

server.o: server.cpp server.h
	$(cc) -c $(cflags) server.cpp

tcpmessenger.o: tcpmessenger.cpp tcpmessenger.h
	$(cc) -c $(cflags) tcpmessenger.cpp

udpmessenger.o: udpmessenger.cpp udpmessenger.h
	$(cc) -c $(cflags) udpmessenger.cpp

server_main.o: server_main.cpp
	$(cc) -c $(cflags) server_main.cpp

client_main.o: client_main.cpp
	$(cc) -c $(cflags) client_main.cpp

clean :
	rm additional_exceptions.o client_state.o client.o events.o helper.o messeges.o server_state.o server.o tcpmessenger.o udpmessenger.o server_main.o client_main.o $(server_exec) $(client_exec)
