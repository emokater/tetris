/**
 * \file back.h
 * \brief Заголовочный файл с определением основных структур и функций для
 * управления игровым процессом.
 */

#ifndef BACK_H
#define BACK_H

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define PIECE_SIZE 4

#include "../../layer/game.h"

/// \brief Возможные состояния игрового цикла.
typedef enum { STATE_START, STATE_GAME, STATE_PAUSE, STATE_EXIT } GameState_t;

/// \brief Структура, описывающая текущую фигуру на поле.
typedef struct {
  int color;
  int x;
  int y;
  int **shape;
} Shape;

/// \brief Основная структура с параметрами игры.
typedef struct {
  GameInfo_t *data;
  GameState_t *state;
  Shape *cur_shape;
} GameParams_t;

void clearField(GameParams_t *params);
void clearShape(GameParams_t *params);
void placeShape(GameParams_t *params);
void setNewShape(int **shape);

int cntEmptyColsL(int **shape);
int cntEmptyColsR(int **shape);
int height(int **target);
int isPossbl(const GameParams_t *params, int **target, int x, int y);
int isSquare(int **shape);
int isVertical(const int target[PIECE_SIZE][PIECE_SIZE]);
void rotate(GameParams_t *params);

int hasCollisBellow(GameParams_t *params);
void spawnNew(GameParams_t *params);
void updtScore(GameParams_t *params, int cnt);
void updtHighScore(GameParams_t *params);
void updtLevel(GameParams_t *params, int *new_lev, int cnt);
void checkLines(GameParams_t *params);
void autoDown(GameParams_t *params);
void down(GameParams_t *params);

void freeMemory(GameParams_t *params);

void setStat(GameParams_t *params);
void setCurShape(GameParams_t *params);
GameParams_t *getParams();

void updtInfo(UserAction_t action);

#endif