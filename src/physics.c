#include "physics.h"
#include <stdbool.h>
#include <string.h>

// Body functions

Vec2 v2(float x, float y) {
  Vec2 result = {x, y};
  return result;
}

void body_init(Body *b, float x, float y) {
  if (x < 0)
    x = 0;
  if (x > (float)(W - 1))
    x = (float)(W - 1);
  if (y < 0)
    y = 0;
  if (y > (float)(H - 1))
    y = (float)(H - 1);

  // Quantize to the simulation grid so occupancy checks stay exact.
  b->pos = v2((float)((int)x), (float)((int)y));
  b->vel = 2.0;
}

int getCellIndex(float pos, int numCells) {
  int idx = (int)(pos / GRID_SIZE);
  if (idx < 0)
    return 0;
  if (idx >= numCells)
    return numCells - 1;
  return idx;
}

static bool grid_in_bounds(int x, int y) {
  return x >= 0 && x < NUM_CELLS_X && y >= 0 && y < NUM_CELLS_Y;
}

static bool cell_is_occupied(GridCell grid[NUM_CELLS_X][NUM_CELLS_Y], int x,
                             int y, int self_index) {
  if (!grid_in_bounds(x, y)) {
    return true;
  }
  int occupant = grid[x][y].body_index;
  return occupant != -1 && occupant != self_index;
}

static void move_body_on_grid(Body *b, int body_index,
                              GridCell grid[NUM_CELLS_X][NUM_CELLS_Y], int newX,
                              int newY) {
  int oldX = getCellIndex(b->pos.x, NUM_CELLS_X);
  int oldY = getCellIndex(b->pos.y, NUM_CELLS_Y);

  if (grid_in_bounds(oldX, oldY) && grid[oldX][oldY].body_index == body_index) {
    grid[oldX][oldY].body_index = -1;
  }

  if (grid_in_bounds(newX, newY)) {
    grid[newX][newY].body_index = body_index;
    b->pos.x = (float)newX;
    b->pos.y = (float)newY;
  }
}

void insertBodyIntoGrid(Body *b, int body_index,
                        GridCell grid[NUM_CELLS_X][NUM_CELLS_Y]) {
  int xIndex = getCellIndex(b->pos.x, NUM_CELLS_X);
  int yIndex = getCellIndex(b->pos.y, NUM_CELLS_Y);

  if (!grid_in_bounds(xIndex, yIndex)) {
    return;
  }
  grid[xIndex][yIndex].body_index = body_index;
}

void updateBodies(Body bodies[], GridCell grid[NUM_CELLS_X][NUM_CELLS_Y],
                  int count) {
  memset(grid, 0xFF, sizeof(GridCell) * NUM_CELLS_X * NUM_CELLS_Y);

  for (int i = 0; i < count; i++) {
    insertBodyIntoGrid(&bodies[i], i, grid);
  }
}

void body_integrate(Body *b, int body_index,
                    GridCell grid[NUM_CELLS_X][NUM_CELLS_Y], float dt,
                    int frameidx) {
  (void)dt;

  int x = getCellIndex(b->pos.x, NUM_CELLS_X);
  int y = getCellIndex(b->pos.y, NUM_CELLS_Y);
  int downY = y + 1;

  if (!grid_in_bounds(x, y)) {
    return;
  }

  if (downY >= NUM_CELLS_Y) {
    return;
  }

  // Fast path: fall straight down when empty.
  if (!cell_is_occupied(grid, x, downY, body_index)) {
    move_body_on_grid(b, body_index, grid, x, downY);
    return;
  }

  // Diagonal slide when blocked below.
  const int preferLeft = (frameidx % 2) == 0;
  const int firstDx = preferLeft ? -1 : 1;
  const int secondDx = -firstDx;

  int tryX = x + firstDx;
  if (!cell_is_occupied(grid, tryX, downY, body_index)) {
    move_body_on_grid(b, body_index, grid, tryX, downY);
    return;
  }

  tryX = x + secondDx;
  if (!cell_is_occupied(grid, tryX, downY, body_index)) {
    move_body_on_grid(b, body_index, grid, tryX, downY);
  }
}

void solve_ground(Body *b, float groundY) {
  if (b->pos.y + BODY_HEIGHT > groundY) {
    b->pos.y = groundY - BODY_HEIGHT;
    b->vel = 0;
  }
}
