SRCS_POLLER = ./server/client_buffer.c ./commonlib/message_peer.c ./commonlib/string_manipulation.c ./server/poll_logging.c ./server/poll_statistics.c ./server/thread_workers.c ./server/poller.c
OBJS_POLLER = $(SRCS_POLLER:.c=.o)

SRCS_CLIENT = ./commonlib/message_peer.c ./client/client_side.c ./client/pollSwayer.c ./commonlib/string_manipulation.c
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)

all : poller pollSwayer

poller: $(OBJS_POLLER)
	gcc -pthread -o poller $(OBJS_POLLER)

pollSwayer: $(OBJS_CLIENT)
	gcc -pthread -o pollSwayer $(OBJS_CLIENT)

client_buffer.o: ./server/client_buffer.c ./server/client_buffer.h
	gcc -c ./server/client_buffer.c -o ./server/client_buffer.o

message_peer.o: ./commonlib/message_peer.c ./commonlib/message_peer.h
	gcc -c ./commonlib/message_peer.c -o ./commonlib/message_peer.o

string_manipulation.o: ./commonlib/string_manipulation.c ./commonlib/string_manipulation.h
	gcc -c ./commonlib/string_manipulation.c -o ./commonlib/string_manipulation.o

poll_statistics.o: ./server/poll_statistics.c ./server/poll_statistics.h
	gcc -c ./server/poll_statistics.c -o ./server/poll_statistics.o

poll_logging.o: ./server/poll_logging.c ./server/poll_logging.h ./commonlib/string_manipulation.h
	gcc -c ./server/poll_logging.c -o ./server/poll_logging.o

thread_workers.o: ./server/thread_workers.h  ./server/thread_workers.c ./server/client_buffer.h ./commonlib/message_peer.h ./commonlib/string_manipulation.h ./server/poll_logging.h ./server/poll_statistics.h
	gcc -c ./server/thread_workers.c -o ./server/thread_workers.o

poller.o: ./server/poller.c ./server/thread_workers.h ./server/client_buffer.h ./server/poll_statistics.h
	gcc -c ./server/poller.c -o ./server/poller.o

client_side.o: ./client/client_side.c ./client/client_side.h ./commonlib/message_peer.h ./commonlib/string_manipulation.h
	gcc -c ./client/client_side.c -o ./client/client_side.o

pollSwayer.o: ./client/pollSwayer.c ./client/client_side.h 
	gcc -c ./client/pollSwayer.c -o ./client/pollSwayer.o

.PHONY: clean
clean:
	rm -f poller pollSwayer pollLog.txt pollStats.txt $(OBJS_POLLER) $(OBJS_CLIENT)



