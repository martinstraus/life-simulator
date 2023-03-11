build:
	gcc -o life geometry.c life.c -lGL -lGLU -lglut -lGLEW

clean:
	rm life
