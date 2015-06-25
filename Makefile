INCLUDE_DIR=$(shell echo ~)/local/include
LIBRARY_DIR=$(shell echo ~)/local/lib
DESDTIR=/
PREFIX=/usr

CXX=clang++
CXXFLAGS=-L${LIBRARY_DIR} -I${INCLUDE_DIR} -g -std=c++11 -fPIC -Wall -Wextra -O2 -march=native

all: libraries

install: libraries
	mkdir -p ${DESTDIR}/${PREFIX}/lib
	mkdir -p ${DESTDIR}/${PREFIX}/include
	cp *.a ${DESTDIR}/${PREFIX}/lib
	cp *.so ${DESTDIR}/${PREFIX}/lib
	cp src/*.h ${DESTDIR}/${PREFIX}/include

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

clean:
	rm -rf test/
	rm -rf *.o
	rm -rf *.a
	rm -rf *.so
