# Room-Game
This is a project created for my Operating Systems class. It is built in C. Buildrooms.c creates a set of rooms which will be navigated by the user in Adventure.c.

To play run:

gcc -o nelsonc4.buildrooms nelsonc4.buildrooms.c

./nelsonc4.buildrooms

This creates a randomly generated room directory needed to play the game.



Then run:

gcc -o nelsonc4.adventure nelsonc4.adventure -lpthread

./nelsonc4.adventure
