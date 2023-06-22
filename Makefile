build:
	gcc -g -o life life.c -lGL -lGLU -lglut -lGLEW -lm
	gcc -g -o nn nn.c -lm

clean:
	rm life
