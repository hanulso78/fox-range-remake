#   DJGPP makefile for Jptui Demo

FLAGS = 

all : jpdemo.exe 

jpdemo.exe : jpdemo.cpp
	g++ $(FLAGS) -I../jptui -o $@ $< ../jptui/libjptui.a 2>&1 | tee -a compile.log

clean:
	rm -f jpdemo.exe
