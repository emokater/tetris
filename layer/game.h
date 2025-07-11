/**
 * @file game.h
 * @brief Определения пользовательских действий и структуры состояния игры.
 */

#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

/**
 * @brief Возможные действия пользователя в игре.
 * Перечисление включает команды управления фигурой,
 * а также управление состоянием игры.
 */
typedef enum {
  Start,
  Pause,
  Terminate,
  Left,
  Right,
  Up,
  Down,
  Action
} UserAction_t;

/**
 * @brief Структура для хранения текущего состояния игры.
 * Содержит игровое поле, буфер следующей фигуры и параметры прогресса.
 */
typedef struct {
  int **field;
  int **next;
  int score;
  int high_score;
  int level;
  int speed;
  int pause;
} GameInfo_t;

void userInput(UserAction_t action, bool hold);

GameInfo_t updateCurrentState();

#endif