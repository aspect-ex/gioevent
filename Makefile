TARGET = gioevent
OBJS = gioevent.o beaglelib.o
INCLUDE = `pkg-config glib-2.0 --cflags`
LIBS = -lglib-2.0 -lrt

${TARGET}: ${OBJS}
	${CXX} -o ${TARGET} $^ ${LIBS}
 
.cpp.o:
	#C++14対応
	${CXX} -std=c++1y -c $< ${INCLUDE}

clean:
	rm ./*.o ${TARGET}

