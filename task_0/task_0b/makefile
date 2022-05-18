all: mypipeline



mypipeline: mypipeline.c LineParser.c
	gcc -g -m32 -c -o mypipeline.o mypipeline.c
	gcc -g -m32 -c -o LineParser.o LineParser.c
	gcc -g -m32  mypipeline.o LineParser.o -o mypipeline
	

.PHONY: clean
clean:
	rm -rf ./*.o mypipeline

