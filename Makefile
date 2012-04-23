all: test1 test2 test3 test4 test5 test6

test1: test1.c
	gcc test1.c -o test1 -lOpenCL -I/opt/AMDAPP/include -L/opt/AMDAPP/lib

test2: test2.c
	gcc test2.c -o test2 -lOpenCL -I/opt/AMDAPP/include -L/opt/AMDAPP/lib

test3: test3.c
	gcc test3.c -o test3 -lOpenCL -I/opt/AMDAPP/include -L/opt/AMDAPP/lib

test4: test4.c
	gcc test4.c -o test4 -lOpenCL -lm -I/opt/AMDAPP/include -L/opt/AMDAPP/lib

test5: test5.c
	gcc test5.c -o test5 -lOpenCL -I/opt/AMDAPP/include -L/opt/AMDAPP/lib

test6: test6.c
	gcc test6.c -o test6 -lm

clean:
	rm -f test1 test2 test3 test4 test5 test6

