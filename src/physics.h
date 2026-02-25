#ifndef PHYSICS_H
#define PHYSICS_H
#include "config.h"

typedef struct {
  float x, y;
} Vec2;

typedef struct {
  Vec2 pos;
  float vel;
} Body;

typedef struct {
  int body_index; // -1 means empty
} GridCell;

Vec2 v2(float x, float y);

void body_init(Body *b, float x, float y);
void body_integrate(Body *b, int body_index,
                    GridCell grid[NUM_CELLS_X][NUM_CELLS_Y], float dt,
                    int frameidx);

void solve_ground(Body *b, float groundY);

int getCellIndex(float pos, int numCells);

void insertBodyIntoGrid(Body *b, int body_index,
                        GridCell grid[NUM_CELLS_X][NUM_CELLS_Y]);

void updateBodies(Body bodies[], GridCell grid[NUM_CELLS_X][NUM_CELLS_Y],
                  int count);

#endif
