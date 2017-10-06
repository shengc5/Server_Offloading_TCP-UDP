TARGET = client_compile edge_compile server_and_compile server_or_compile
all : $(TARGET)
client_compile : client.c
	gcc -o client client.c
edge_compile : edge.c
	gcc -o edge.o edge.c
server_and_compile : server_and.c
	gcc -o server_and.o server_and.c
server_or_compile : server_or.c
	gcc -o server_or.o server_or.c
edge : 
	./edge.o
server_and :
	./server_and.o
server_or :
	./server_or.o