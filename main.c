#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

typedef struct {
  char** board;
  int x_pos;
  int y_pos;
  ssize_t move_number;
} tour_args;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////DEBUGGING REMINDER///////////////////////////////////////////////////
// (1) Check for "Dealloc reminder" -> these functions potentially allocate memory that needs to be
//     deallocated by caller of function.

// define global variables
int MAX_TOUR_SIZE = 0;
char*** dead_end_tours = NULL;
int dead_end_count = 0;

int board_width;
int board_height;

// define mutexes
pthread_mutex_t tour_size_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t dead_end_tours_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_board (char** board, int width, int height);
void copy_board (char** board, char** destination);
void tour (char** board_, int x_pos, int y_pos, ssize_t move_number);
int fill_count (char** board);
void tour (void* args);

int main (int argc, char** argv) {

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////SETUP//////////////////////////////////////////////////////
  ///////////////////////////////////////////ARGUMENT PARSING/////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // need at least 3 args
  if ( argc < 3 ) {
    fprintf(stderr, "Error: Invalid number of arguments.\n");
    return 1;
  }

  // parse the arguments
  board_width = atoi(argv[1]);
  board_height = atoi(argv[2]);

  if ( board_width <= 0 || board_height <= 0 ) {
    fprintf(stderr, "Error: Invalid board dimensions. \n");
    return 1;
  }

  // successfully set up variables
  printf ("THREAD %li: Solving Sonny's knight's tour problem for a %dx%d board\n", pthread_self(), board_width, board_height);

  // initialize board
  // [Dealloc reminder!]
  char** chess_board;
  init_board (chess_board, board_width, board_height);

  // start the tour...
  tour (chess_board, 0, 0, 0);

  return 0;
}

void tour (void* args) {

  // beginning of thread
  tour_args* arguments = (tour_args*) args;
  tour (arguments->board, arguments->x_pos, arguments->y_pos, arguments->move_number);

  // exit the thread -- return the move number
  pthread_exit ((void *) tour_args->move_number);
}

void tour (char** board_, int x_pos, int y_pos, ssize_t move_number) {
  // fill the x_pos and y_pos on the board

  // copy board then fill TODO: copy!!!
  char** board;
  copy_board (board_, board_width, board_height, board);
  fill_board (board, x_pos, y_pos, 1);

  move_number += 1;
  int* possible_positions;
  int possible_moves_ = possible_moves (board, x_pos, y_pos, possible_positions);
  void* thread_return_val;

  if ( possible_moves_ > 0 ) {
    // log possible positions
    printf ("THREAD %li: %d moves possible after move #%d; creating threads\n", pthread_self(), possible_moves, move_number);
    thread_t tid_arr[possible_moves];

    for (int i = 0; i < possible_moves; ++i){
      // create all the threads
      tour_args new_args;
      new_args->board = board;
      // row, then col, in possible_positions
      new_args->y_pos = *(possible_positions+(2*i));
      new_args->x_pos = *(possible_positions+(2*i+1));
      new_args->move_number = move_number + 1;

      if (pthread_create ( &(tid_arr[i]), NULL, tour, (void*) &new_args ) != 0 ){
        fprintf (stderr, "Error: could not create thread... terminating.\n");
        exit (2);
      }
    } // end of for
  }

  // join all the threads created back with the instantiating thread...
  for (int i = 0; i < possible_moves; ++i) {
    pthread_join (tid_arr[i], &thread_return_val);
    printf ("Thread [%li] joined (returned %d)\n", tid_arr[i], thread_return_val);
  }

  else {
    // no moves possible -- dont create thread
    // determine if dead end or full knight
    if (fill_count(board) == board_width * board_height) {
      // board filled
      printf ("Thread %li: Sonny found a full knight's tour!\n", pthread_self());
    }
    else {
      printf ("Thread %li: Dead end after move #%d\n", pthread_self(), move_number);
    }
  }

} // end of tour

int fill_count (char** board) {
  // return the count of the positions reached by the knight.
  int count_ = 0;
  for ( int i = 0; i < board_height; ++i ) {
    for ( int j = 0; j < board_width; ++j ) {
      // 0 means empty
      if (board[i][j] != 0)
        ++ count_;
    }
  }

  return count_;
}

void init_board (char** board, int width, int height) {
  // initialize a new board of dimensions width x height and return it into board pointer
  // [Dealloc reminder!]

  board = (char**) calloc (height, sizeof(char*));
  for (int i = 0; i < height; ++i)
    *(board+i) = (char*) calloc (width, sizeof(char));
}

void copy_board (char** board, int width, int height, char** destination) {
  init_board (destination, width, height);

  for ( int row_ = 0; row_ < height; ++ row_ ) {
    for ( int col_ = 0; col_ < width; ++ col_ ) {
      destination[row_][col_] = board[row_][col_];
    }
  }
}

bool in_bounds (char** board, int col, int row) {
  return col < board_width && col >= 0
      && row < board_height && row >= 0;
}

bool is_filled (char** board, int col, int row) {
  // 0 => empty
  // 1 => filled
  // 2 => current_pos
  char* board_row = *(board+row);
  return *( board_row+col ) != 0; // 1 or 2 means filled; 0 means empty
}

bool fill_board (char** board, int col, int row, char fill_char) {
  assert (col < board_width);
  assert (col >= 0);
  assert (row < board_height);
  assert (row >= 0);

  if ( is_filled (board, col, row) ) return false;
  *(*(board+row)+col) = fill_char;

  return true;
}

int possible_moves (char** current_board, int x_pos, int y_pos, int* possible_positions) {
  // given a current board with an x and y position, determine which positions the knight can occupy as a next move (and store in possible_positions)
  // and return the amount of possible moves found.

  // [Dealloc reminder!]
  int moves[] = {1, 2,
                 1, -2,
                 -1, 2,
                 -1, -2,
                 2, 1,
                 2, -1,
                 -2, 1,
                 -2, -1};

  int i, next_row, next_col;
  int possible_moves_ = 0;
  possible_positions = (int*) calloc (16, sizeof(int));

  for ( i = 0; i < 16; i+=2 ) {
    next_row = y_pos + *(moves+i);
    next_col = x_pos + *(moves+i+1);

    // check if the position is free to occupy
    if ( in_bounds(current_board, next_col, next_row) && !is_filled(current_board, next_col, next_row) ) {
      *(possible_positions+(2*possible_moves_) ) = next_row;
      *(possible_positions+(2*possible_moves_+1) ) = next_col;
      ++ possible_moves_;
    }
  }

  // shorten the size of possible_positions
  possible_positions = (int*) realloc (possible_positions, possible_moves_ * 2);

  return possible_moves_;
}
