/**
 * @file game.c
 * @brief Интерфейс между пользовательским вводом и движком игры tetris.
 * Содержит функции для обработки ввода пользователя и получения текущего
 * состояния игры.
 */

#include "game.h"

#include <stdio.h>

#include "../brick_game/tetris/back.h"

/**
 * @brief Обрабатывает действие пользователя и обновляет состояние игры.
 * @param action Тип действия пользователя (UserAction_t):
 *        Start, Pause, Left, Right, Up, Down, Action, Terminate.
 * @param hold   Логический флаг удержания клавиши.
 */
void userInput(UserAction_t action, bool hold) {
  (void)hold;
  updtInfo(action);
}

/**
 * @brief Возвращает текущее состояние игры.
 * Получает указатель на структуру GameInfo_t через getParams(),
 * копирует её содержимое и возвращает по значению.
 *
 * @return текущее состояние игры GameInfo_t.
 */
GameInfo_t updateCurrentState() {
  const GameInfo_t *cur = getParams()->data;
  return *cur;
}