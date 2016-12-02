include ../../../xia.mk

.PHONY: clean

LDFLAGS += $(LIBS)
TARGETS = stage_utils.o stageManager stageServer xping testXping video_publisher manifest_server proxy
CFLAGS += -I/usr/include/libxml2

all: $(TARGETS)
	rm stage_utils.o

stage_utils.o:
	$(CC) $(CFLAGS) -g -I../../../click/include/click -std=c++11 -c stage_utils.c

csapp.o: csapp.c csapp.h
		$(CC) $(CFLAGS) -Wno-write-strings -Wno-sign-compare -c csapp.c -std=c++11

manifest.o: manifest.c manifest.h
		$(CC) $(CFLAGS) -I/usr/local/include/libxml2 -Wno-write-strings -Wno-sign-compare -c manifest.c -std=c++11

utils.o: utils.c utils.h
		$(CC) $(CFLAGS) -Wno-write-strings -Wno-sign-compare -c utils.c

video_publisher.o: video_publisher.c video_publisher.h utils.h manifest.h
		$(CC) $(CFLAGS) -c video_publisher.c -I/usr/local/include/libxml2 -Wno-unused-parameter -std=c++11

manifest_server.o: manifest_server.c manifest_server.h utils.h 
		$(CC) $(CFLAGS) -c manifest_server.c -std=c++11

proxy.o: proxy.c proxy.h csapp.h stage_utils.h
		$(CC) $(CFLAGS) -Wno-write-strings -Wno-sign-compare -c proxy.c -std=c++11

stageManager:
	$(CC) $(CFLAGS) stage_manager.c stage_utils.o -o stage_manager $(LDFLAGS) -lpthread -lprotobuf

stageServer:
	$(CC) $(CFLAGS) stage_server.c stage_utils.o -o stage_server $(LDFLAGS) -lpthread -lprotobuf
	
xping:
	$(CC) $(CFLAGS) xping.c -o xping $(LDFLAGS) -lpthread -lprotobuf

testXping:
	$(CC) $(CFLAGS) testXping.c stage_utils.o -o testXping $(LDFLAGS) -lpthread -lprotobuf
		
proxy: proxy.o csapp.o stage_utils.o $(LDFLAGS) -lprotobuf -lpthread

video_publisher: video_publisher.o manifest.o utils.o $(LDFLAGS) -lxml2 -lprotobuf -lpthread

manifest_server: manifest_server.o utils.o $(LDFLAGS) -lprotobuf -lpthread

clean:
	-rm -f *.o
	-rm -f *.mpd
	-rm -f *.txt
	-rm -f video_publisher
	-rm -f manifest_server
	-rm -f proxy
	-rm -f stage_server
	-rm -f stage_manager
	-rm -f xping
	-rm -f testXping
