INCLUDE_DIR=$(shell echo ~)/local/include
LIBRARY_DIR=$(shell echo ~)/local/lib
DESDTIR=/
PREFIX=/usr

CXX=clang++
CXXFLAGS=-L${LIBRARY_DIR} -I${INCLUDE_DIR} -g -std=c++11 -fPIC -Wall -Wextra -O2 -march=native

all: libraries

test: test_server test_client

install: libraries
	mkdir -p ${DESTDIR}/${PREFIX}/lib
	mkdir -p ${DESTDIR}/${PREFIX}/include
	cp *.a ${DESTDIR}/${PREFIX}/lib
	cp *.so ${DESTDIR}/${PREFIX}/lib
	cp src/smplsocket.h ${DESTDIR}/${PREFIX}/include

uninstall:
	rm ${DESTDIR}/${PREFIX}/lib/libsmplsocket.so
	rm ${DESTDIR}/${PREFIX}/lib/libsmplsocket.a
	rm ${DESTDIR}/${PREFIX}/include/smplsocket.h

libraries: libsmplsocket.so libsmplsocket.a

libsmplsocket.so: file_descriptor.o unix_domain_socket.o network_socket.o
	${CXX} ${CXXFLAGS} -shared -Wl,-soname,libsmplsocket.so -o libsmplsocket.so file_descriptor.o unix_domain_socket.o network_socket.o

libsmplsocket.a: file_descriptor.o unix_domain_socket.o network_socket.o
	ar rcs libsmplsocket.a file_descriptor.o unix_domain_socket.o network_socket.o

file_descriptor.o: src/file_descriptor.cc
	${CXX} ${CXXFLAGS} -c src/file_descriptor.cc -o file_descriptor.o

unix_domain_socket.o: src/unix_domain_socket.cc
	${CXX} ${CXXFLAGS} -c src/unix_domain_socket.cc -o unix_domain_socket.o

network_socket.o: src/network_socket.cc
	${CXX} ${CXXFLAGS} -c src/network_socket.cc -o network_socket.o

test_server: src/test_server.cc
	${CXX} ${CXXFLAGS} src/test_server.cc -o test_server -lsmplsocket

test_client: src/test_client.cc
	${CXX} ${CXXFLAGS} src/test_client.cc -o test_client -lsmplsocket

clean:
	rm -rf test_server
	rm -rf test_client
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so
