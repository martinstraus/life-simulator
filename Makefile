build:
	gcc -g -o life life.c -lGL -lGLU -lglut -lGLEW -lm

clean:
	rm life
