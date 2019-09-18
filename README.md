# Knight's Tour Algorithm
This is an implementation of the Knight's tour algorithm. This is a multithreaded implementation of the algorithm using pthreads.

# Usage
Compile main.c. The compiled program takes 2 parameters:
./knights_tour.o <board_width> <board_height>
- *board_width (int):* specify the width of the board
- *board_height (int):* specify the height of the board

This program finds all possible knight tours in a board with given dimensions `board_width x board_height` and reports the number of dead ends and completed tours found.
