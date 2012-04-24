all: test1 test2 test3 test4 test5 test6

test1: test1.c
	nvcc test1.c -o test1 -lOpenCL 

test2: test2.c
	nvcc test2.c -o test2 -lOpenCL 

test3: test3.c
	nvcc test3.c -o test3 -lOpenCL
 
test4: test4.c
	nvcc test4.c -o test4 -lOpenCL 

test5: test5.c
	nvcc test5.c -o test5 -lOpenCL 

test6: test6.c
	gcc test6.c -o test6 -lm

clean:
	rm -f test1 test2 test3 test4 test5 test6

