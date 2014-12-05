#include <ncurses.h>

#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <algorithm>


const int MIN_HEIGHT = 2;
const int MIN_WIDTH = 2;
// Function for creating an open box.
void create_box(int y, int x, int w, int h)
{
  mvaddch(y, x, ' ');
  mvaddch(y, x + w, ' ');
  mvaddch(y + h, x, ' ');
  mvaddch(y + h, x + w, ' ');
  mvhline(y, x + 1, ' ', w - 1);
  mvhline(y + h, x + 1, ' ', w - 1);
  mvvline(y + 1, x, ' ', h - 1);
  mvvline(y + 1, x + w, ' ', h - 1);
}

// Function for creating a filled box.
void fill_rect(int y, int x, int w, int h)
{
  mvhline(y, x, ' ', w);
  for (int n = 1; n < h; n++)
    mvhline(y+n, x, ' ', w);
}


void initialize_drawing() {
  // Start the ncurses control of the screen. Since only one window is used, it will take the whole screen.
  initscr();

  // Seed the random number generator with the current time, to achieve a good degree of randomness.
  srand((unsigned)time(0));

  // Configure the screen. I'm not sure what all these do.
  start_color();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();

  // Set up color pairs, for use when drawing ncurses text.
  init_pair(0, COLOR_WHITE, COLOR_WHITE);
  init_pair(1, COLOR_RED, COLOR_RED);
  init_pair(2, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(3, COLOR_GREEN, COLOR_GREEN);
  init_pair(4, COLOR_CYAN, COLOR_CYAN);
  init_pair(5, COLOR_BLUE, COLOR_BLUE);
  init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(7, COLOR_BLACK, COLOR_BLACK);
}

void draw_block(int xmultiplier, int ymultiplier, int w, int h, int type) {
  int colors = 8;
  // Declare variables for the position of the tiles.
  int startx, starty, height, width, color;

  color = rand() % colors;

  //height = rand() % LINES/2;
  //width = rand() % COLS/2;
  //starty = (LINES - height)/2;
  //startx = (COLS - width)/2;

  starty = rand() % LINES;
  startx = rand() % COLS;

  //height = MIN_HEIGHT;
  //width = MIN_WIDTH;

  height = 1 * std::max(MIN_HEIGHT,(int)round(ymultiplier * MIN_HEIGHT/std::min(MIN_HEIGHT,MIN_WIDTH)*h/256));
  width = 2 * std::max(MIN_WIDTH, (int)round(xmultiplier * MIN_WIDTH/std::min(MIN_HEIGHT,MIN_WIDTH)*w/256));

  attron(COLOR_PAIR(color));


  //create_box(starty, startx, width, height));
  fill_rect(starty, startx, width, height);
}

void uninitialize_drawing() {
  getch();

  endwin();
}

void refresh_drawing() {
  refresh();
}
