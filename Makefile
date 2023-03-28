build:
	gcc -o life geometry.c world.c life.c render.c -lGL -lGLU -lglut -lGLEW

clean:
	rm life
