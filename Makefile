vice-project-doom: main.o enemies.o level.o player.o powerups.o scores.o
	g++ main.o enemies.o level.o player.o powerups.o scores.o -o vice-project-doom -lsfml-graphics -lsfml-audio -lsfml-window -lsfml-system

main.o: src/main.cpp
	g++ -c src/main.cpp

enemies.o: src/enemies.cpp
	g++ -c src/enemies.cpp

level.o: src/level.cpp
	g++ -c src/level.cpp

player.o: src/player.cpp
	g++ -c src/player.cpp

powerups.o: src/powerups.cpp
	g++ -c src/powerups.cpp

scores.o: src/scores.cpp
	g++ -c src/scores.cpp

clean:
	rm -rf *.o vice-project-doom
