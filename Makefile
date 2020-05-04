CC = g++ -std=c++14
CFLAGS = -Wall -Wextra -Wpedantic -Ofast

build: subscriber server

subscriber: ./obj ./obj/validator.o ./obj/broker.o ./obj/header.o ./obj/utils.o ./obj/subscriber.o
	$(CC) $(CFLAGS) ./src/subscriber/main.cc ./obj/*.o -o subscriber

server: ./obj/validator.o ./obj/broker.o ./obj/header.o ./obj/utils.o ./obj/server.o
	$(CC) $(CFLAGS) ./src/server/main.cc ./obj/*.o -o server

./obj/validator.o: 
	$(CC) $(CFLAGS) -c ./src/validator/validator.cc -o ./obj/validator.o

./obj/broker.o: ./src/protocol/tcp_broker.cc
	$(CC) $(CFLAGS) -c ./src/protocol/tcp_broker.cc -o ./obj/broker.o

./obj/header.o: ./src/protocol/tcp_header.cc
	$(CC) $(CFLAGS) -c ./src/protocol/tcp_header.cc -o ./obj/header.o

./obj/utils.o: ./src/utils/utils.cc
	$(CC) $(CFLAGS) -c ./src/utils/utils.cc -o ./obj/utils.o

./obj/subscriber.o: ./src/subscriber/subscriber.cc
	$(CC) $(CFLAGS) -c ./src/subscriber/subscriber.cc -o ./obj/subscriber.o

./obj/server.o: ./src/server/server.cc
	$(CC) $(CFLAGS) -c ./src/server/server.cc -o ./obj/server.o

./obj:
	mkdir -p ./obj

clean:
	rm -rf server subscriber obj
