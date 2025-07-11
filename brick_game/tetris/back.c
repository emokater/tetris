/*!
 * \file back.c
 * \brief Реализация логики работы игры тетрис.
 *
 * Содержит функции для очистки поля, управления памятью, установки и
 * перемещения фигур, обновления очков и уровней, проверки столкновений.
 */

#include "back.h"

#include <stdio.h>  /**< Для работы с NULL и файловыми функциями */
#include <stdlib.h> /**< Для malloc, calloc, free, rand */
#include <stdlib.h>
#include <time.h>

/**
 * \brief Полностью очищает игровое поле, устанавливая все ячейки в 0.
 * \param params Указатель на структуру параметров игры.
 */
void clearField(GameParams_t *params) {
  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    for (int j = 0; j < FIELD_WIDTH; ++j) {
      params->data->field[i][j] = 0;
    }
  }
}

/**
 * \brief Удаляет текущую фигуру с поля, устанавливая соответствующие ячейки в
 * 0.
 * \param params Указатель на структуру параметров игры.
 */
void clearShape(GameParams_t *params) {
  int x = params->cur_shape->x;
  int y = params->cur_shape->y;
  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE; ++j) {
      if (params->cur_shape->shape[i][j] != 0) {
        params->data->field[y + i][x + j] = 0;
      }
    }
  }
}

/**
 * \brief Размещает текущую фигуру на игровом поле.
 * \param params Указатель на структуру параметров игры.
 */
void placeShape(GameParams_t *params) {
  int x = params->cur_shape->x;
  int y = params->cur_shape->y;

  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE; ++j) {
      if (params->cur_shape->shape[i][j] != 0) {
        params->data->field[i + y][j + x] =
            params->cur_shape->shape[i][j] * params->cur_shape->color;
      }
    }
  }
}

/**
 * \brief Генерирует новую фигуру из набора стандартных семи случайным образом.
 * \param shape Инициализируемый двумерный массив размера PIECE_SIZE.
 */
void setNewShape(int **shape) {
  const int shapes[7][PIECE_SIZE][PIECE_SIZE] = {
      {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},

      {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},

      {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},

      {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},

      {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},

      {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},

      {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

  int num_shape = rand() % 7;

  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE; ++j) {
      shape[i][j] = shapes[num_shape][i][j];
    }
  }
}

/**
 * \brief Подсчитывает пустые столбцы слева от фигуры.
 * \param shape Двумерный массив фигуры.
 * \return Количество полностью пустых столбцов слева.
 */
int cntEmptyColsL(int **shape) {
  int flag = 1;
  int cnt = 0;

  for (int i = 0; i < PIECE_SIZE && flag; ++i) {
    for (int j = 0; j < PIECE_SIZE && flag; ++j) {
      if (shape[j][i] != 0) {
        flag = 0;
      }
    }

    if (flag) {
      cnt += 1;
    }
  }

  return cnt;
}

/**
 * \brief Подсчитывает количество полностью пустых столбцов справа от фигуры.
 * \param shape Двумерный массив фигуры.
 * \return Количество пустых столбцов справа.
 */
int cntEmptyColsR(int **shape) {
  int flag = 1;
  int cnt = 0;

  for (int i = 3; i >= 0 && flag; --i) {
    for (int j = 0; j < PIECE_SIZE && flag; ++j) {
      if (shape[j][i] != 0) {
        flag = 0;
      }
    }

    if (flag) {
      cnt += 1;
    }
  }

  return cnt;
}

/**
 * \brief Вычисляет высоту фигуры.
 * Используется для корректной проверки выхода фигуры за нижнюю границу поля (в
 * isPossbl()).
 *
 * \param target Двумерный массив фигуры.
 * \return Высота фигуры в строках.
 */
int height(int **target) {
  int h = 0;
  int flag = 0;

  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE; ++j) {
      if (target[i][j] != 0) {
        flag = 1;
      }
    }
    if (flag) {
      h += 1;
      flag = 0;
    }
  }

  return h;
}

/**
 * \brief Проверяет, можно ли разместить фигуру в позиции (x,y).
 *  Выполняет проверку двух основных условий: границы поля и пересечение с уже
 * заполненными ячейками.
 *
 * \param params Параметры игры.
 * \param target Фигура.
 * \param x Координата x на поле.
 * \param y Координата y на поле.
 * \return 1, если возможно, иначе 0.
 */
int isPossbl(const GameParams_t *params, int **target, int x, int y) {
  int res = 1;

  if (y + height(target) - 1 > FIELD_HEIGHT - 1 ||
      x + 3 - cntEmptyColsR(target) > FIELD_WIDTH - 1 ||
      x + cntEmptyColsL(target) < 0) {
    res = 0;
  }

  for (int i = 0; i < PIECE_SIZE && res; ++i) {
    for (int j = 0; j < PIECE_SIZE && res; ++j) {
      if (target[i][j] != 0 && params->data->field[i + y][j + x] != 0) {
        res = 0;
      }
    }
  }

  return res;
}

/**
 * \brief Проверяет, является ли фигура квадратом.
 * Нужна для того, чтобы не поварачивать фигуру,
 * если она является квадратом, так как это не имеет смысла.
 *
 * \param shape Фигура.
 * \return 1, если квадрат, иначе 0.
 */
int isSquare(int **shape) {
  int res = 0;
  if (shape[0][1] != 0 && shape[0][2] != 0 && shape[1][1] != 0 &&
      shape[1][2] != 0) {
    res = 1;
  }
  return res;
}

/**
 * \brief Проверяет, является ли фигура вертикальной линией.
 * Нужна для более корректной отрисовки фигуры после поворота.
 *
 * \param shape Фигура.
 * \return 1, если да, иначе 0.
 */
int isVertical(const int shape[PIECE_SIZE][PIECE_SIZE]) {
  int res = 0;
  int cnt = 0;

  for (int x = 0; x < PIECE_SIZE && !res; ++x) {
    for (int i = 0; i < PIECE_SIZE; ++i) {
      if (shape[i][x] != 0) {
        cnt += 1;
      }
    }

    if (cnt == 4) {
      res = 1;
    } else {
      cnt = 0;
    }
  }

  return res;
}

/**
 * @brief Сдвигает символы в массиве после переворота для более красивого
 * отображения в поле
 *
 * @param tmp Массив с перевернутой фигурой
 * @param new Массив, куда запишется корректно перевернутая фигура
 */
static void shiftAfterRotation(int tmp[PIECE_SIZE][PIECE_SIZE], int **new) {
  if (isVertical(tmp)) {
    for (int i = 0; i < PIECE_SIZE; ++i) {
      for (int j = 0; j < PIECE_SIZE; ++j) {
        if (j == 1) {
          new[i][j] = 1;
        } else {
          new[i][j] = 0;
        }
      }
    }
  } else {
    for (int i = 0; i < PIECE_SIZE; ++i) {
      for (int j = 0; j < PIECE_SIZE; ++j) {
        if (i == PIECE_SIZE - 1) {
          new[i][j] = 0;
        } else {
          new[i][j] = tmp[i + 1][j];
        }
      }
    }
  }
}

/**
 * @brief Поворачивает текущую фигуру на 90° по часовой стрелке, если это
 * возможно.
 *
 * Пошаговый алгоритм:
 *   1. Удаляет фигуру с поля (clearShape()).
 *   2. Формирует временный массив new с повёрнутым на 90° представлением
 * фигуры.
 *   3. Создаёт буфер cur, в который копирует new, сдвинув её вверх на одну
 * строку для корректного выравнивания.
 *   4. Проверяет возможность размещения cur на текущей позиции (isPossbl()).
 *   5. При допустимости поворота обновляет shape:
 *        - для вертикальной фигуры применяет new без сдвига.
 *        - для остальных фигур применяет сдвинутый cur.
 *   6. Освобождает память, занятую буфером cur.
 *   7. Размещает обновлённую фигуру на поле (placeShape()).
 *
 * @param params Указатель на структуру с текущим состоянием и данными игры.
 */
void rotate(GameParams_t *params) {
  clearShape(params);
  int tmp[PIECE_SIZE][PIECE_SIZE];
  int **new;
  new = calloc(PIECE_SIZE, sizeof(int *));

  if (!new) {
    perror("calloc new[] failed");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; i < PIECE_SIZE; i++) {
    new[i] = calloc(PIECE_SIZE, sizeof(int));
  }
  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE; ++j) {
      tmp[PIECE_SIZE - 1 - j][i] = params->cur_shape->shape[i][j];
    }
  }

  shiftAfterRotation(tmp, new);

  if (isPossbl(params, new, params->cur_shape->x, params->cur_shape->y)) {
    for (int i = 0; i < PIECE_SIZE; ++i) {
      for (int j = 0; j < PIECE_SIZE; ++j) {
        params->cur_shape->shape[i][j] = new[i][j];
      }
    }
  }
  for (int i = 0; i < PIECE_SIZE; i++) {
    free(new[i]);
    new[i] = NULL;
  }
  free(new);
  placeShape(params);
}

/**
 * @brief Проверяет, происходит ли столкновение при движении фигуры по вертикали
 * вниз. Используется при автоматическом падении (autoDown) и мгновенном сбросе
 * вниз (down) для прекращения движения фигуры и фиксации её на месте.
 *
 * @param params Указатель на структуру с текущими параметрами и состоянием
 * игры.
 * @return 1, если под любой частью фигуры есть препятствие (столкновение),
 * иначе 0.
 */
int hasCollisBellow(GameParams_t *params) {
  int fy, fx;
  int res = 0;
  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE && !res; ++j) {
      if (params->cur_shape->shape[i][j] != 0) {
        fy = params->cur_shape->y + i + 1;
        fx = params->cur_shape->x + j;

        if (fy >= FIELD_HEIGHT || params->data->field[fy][fx] != 0) {
          res = 1;
        }
      }
    }
  }
  return res;
}

/**
 * \brief Размещает новую фигуру из буфера next на место текущей фигуры и
 * генерирует следующую.
 * \param params Параметры игры.
 */
void spawnNew(GameParams_t *params) {
  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE; ++j) {
      params->cur_shape->shape[i][j] = params->data->next[i][j];
    }
  }
  params->cur_shape->x = 3;
  params->cur_shape->y = 0;
  params->cur_shape->color = (rand() % 7) + 1;

  if (isPossbl(params, params->cur_shape->shape, params->cur_shape->x,
               params->cur_shape->y)) {  // костыль против спавна новый х2
    setNewShape(params->data->next);
  }
}

/**
 * \brief Обновляет счёт в зависимости от количества уничтоженных линий.
 * \param params Параметры игры.
 * \param cnt Количество удалённых строк за ход.
 */
void updtScore(GameParams_t *params, int cnt) {
  if (cnt == 1) {
    params->data->score += 100;
  } else if (cnt == 2) {
    params->data->score += 300;
  } else if (cnt == 3) {
    params->data->score += 700;
  } else if (cnt >= 4) {
    params->data->score += 1500;
  }
}

/**
 * \brief Обновляет рекордный счёт и сохраняет в файл.
 * \param params Параметры игры.
 */
void updtHighScore(GameParams_t *params) {
  if (params->data->score > params->data->high_score) {
    params->data->high_score = params->data->score;
    FILE *f = fopen("record.txt", "w");
    if (!f) {
      perror("Error creating record.txt");
    } else {
      fprintf(f, "%d\n", params->data->high_score);
      fclose(f);
    }
  }
}

/**
 * \brief Повышает уровень каждые 600 очков и увеличивает скорость.
 * \param params Параметры игры.
 * \param new_lev Текущий порог для повышения уровня.
 * \param cnt Количество удалённых строк.
 */
void updtLevel(GameParams_t *params, int *new_lev, int cnt) {
  if (params->data->score >= *new_lev && cnt != 0 && params->data->level < 10) {
    params->data->level += 1;
    (*new_lev) += 600;
    params->data->speed -= 100;
  }
}

/**
 * \brief Удаляет заполненные линии, сдвигает поле вниз. Обновляет параметры.
 * \param params Параметры игры.
 */
void checkLines(GameParams_t *params) {
  static int new_lev = 600;

  int cnt = 0;
  int flag = 1;
  int indxs[FIELD_HEIGHT];

  for (int y = 0; y < FIELD_HEIGHT; ++y) {
    for (int x = 0; x < FIELD_WIDTH && flag; ++x) {
      if (params->data->field[y][x] == 0) {
        flag = 0;
      }
    }

    if (flag) {
      cnt += 1;
      indxs[cnt - 1] = y;
    } else {
      flag = 1;
    }
  }

  for (int k = cnt - 1; k >= 0; --k) {
    int row = indxs[k];
    for (int x = 0; x < FIELD_WIDTH; ++x) {
      params->data->field[row][x] = 0;
    }

    for (int y = row; y >= 1; --y) {
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        params->data->field[y][x] = params->data->field[y - 1][x];
      }
    }

    for (int c = 0; c < k; ++c) {
      indxs[c] += 1;
    }
  }

  updtScore(params, cnt);
  updtHighScore(params);
  updtLevel(params, &new_lev, cnt);
}

/**
 * \brief Автоматический спуск фигуры на одну линию вниз.
 * \param params Параметры игры.
 */
void autoDown(GameParams_t *params) {
  clearShape(params);

  if (hasCollisBellow(params)) {
    placeShape(params);
    checkLines(params);
    spawnNew(params);
  } else {
    params->cur_shape->y += 1;
  }

  if (isPossbl(params, params->cur_shape->shape, params->cur_shape->x,
               params->cur_shape->y)) {
    placeShape(params);
  } else {
    *(params->state) = STATE_EXIT;
    params->data->pause = 2;
  }
}

/**
 * \brief Мгновенный спуск фигуры вниз до столкновения.
 * \param params Параметры игры.
 */
void down(GameParams_t *params) {
  clearShape(params);
  while (!hasCollisBellow(params)) {
    params->cur_shape->y++;
  }
  placeShape(params);

  checkLines(params);

  spawnNew(params);
  if (isPossbl(params, params->cur_shape->shape, params->cur_shape->x,
               params->cur_shape->y)) {
    placeShape(params);
  } else {
    *(params->state) = STATE_EXIT;
    params->data->pause = 2;
  }
}

/**
 * \brief Освобождает всю выделенную память, связанную с параметрами игры.
 * \param params Указатель на структуру параметров игры.
 */
void freeMemory(GameParams_t *params) {
  if (params) {
    if (params->data && params->data->field) {
      for (int i = 0; i < FIELD_HEIGHT; i++) {
        free(params->data->field[i]);
        params->data->field[i] = NULL;
      }
      free(params->data->field);
      params->data->field = NULL;
    }

    if (params->data && params->data->next) {
      for (int i = 0; i < PIECE_SIZE; i++) {
        free(params->data->next[i]);
        params->data->next[i] = NULL;
      }
      free(params->data->next);
      params->data->next = NULL;
    }

    if (params->data) {
      free(params->data);
      params->data = NULL;
    }

    if (params->cur_shape && params->cur_shape->shape) {
      for (int i = 0; i < PIECE_SIZE; i++) {
        free(params->cur_shape->shape[i]);
        params->cur_shape->shape[i] = NULL;
      }
      free(params->cur_shape->shape);
      params->cur_shape->shape = NULL;
      free(params->cur_shape);
      params->cur_shape = NULL;
    }

    if (params->state) {
      free(params->state);
      params->state = NULL;
    }

    free(params);
  }
}

/**
 * \brief Выдает ошибку, если при выделении памяти возникли ошибки, и подчистит
 * то, что было таки выделено.
 * \param params Указатель на структуру параметров игры.
 */
static void showErr(GameParams_t *params) {
  perror("malloc/calloc failed");
  freeMemory(params);
  exit(EXIT_FAILURE);
}

/**
 * \brief Задаёт начальные параметры игры (очки, рекорд, уровень, скорость,
 * состояние паузы).
 * \param params Указатель на структуру параметров игры.
 */
void setStat(GameParams_t *params) {
  params->data->score = 0;
  params->data->high_score = 0;
  params->data->level = 1;
  params->data->speed = 1000;
  params->data->pause = 0;
}

/**
 * \brief Задает текущую (самую первую) фигуру, её начальные координаты и цвет.
 * \param params Указатель на структуру параметров игры.
 */
void setCurShape(GameParams_t *params) {
  params->cur_shape = malloc(sizeof *(params->cur_shape));
  if (!params->cur_shape) {
    showErr(params);
  }
  params->cur_shape->shape = calloc(PIECE_SIZE, sizeof(int *));
  if (!params->cur_shape->shape) {
    showErr(params);
  }
  for (int i = 0; i < PIECE_SIZE; i++) {
    params->cur_shape->shape[i] = calloc(PIECE_SIZE, sizeof(int));
  }
  setNewShape(params->cur_shape->shape);
  params->cur_shape->x = 3;
  params->cur_shape->y = 0;
  params->cur_shape->color = (rand() % 7) + 1;
}

/**
 * \brief Инициализация и получение глобального указателя на параметры игры.
 * \return Указатель на статический объект GameParams_t.
 */
GameParams_t *getParams() {
  static GameParams_t *params = NULL;

  if (params == NULL) {
    srand((unsigned)time(NULL));

    params = malloc(sizeof *params);
    if (!params) {
      showErr(params);
    }

    params->state = malloc(sizeof *(params->state));
    if (!params->state) {
      showErr(params);
    }
    *(params->state) = STATE_START;

    params->data = malloc(sizeof *(params->data));
    if (!params->data) {
      showErr(params);
    }

    setStat(params);
    setCurShape(params);

    params->data->next = calloc(PIECE_SIZE, sizeof(int *));
    for (int i = 0; i < PIECE_SIZE; i++) {
      params->data->next[i] = calloc(PIECE_SIZE, sizeof(int));
    }
    setNewShape(params->data->next);

    params->data->field = calloc(FIELD_HEIGHT, sizeof(int *));
    for (int i = 0; i < FIELD_HEIGHT; i++) {
      params->data->field[i] = calloc(FIELD_WIDTH, sizeof(int));
    }

    FILE *f = fopen("record.txt", "r");
    if (f) {
      fscanf(f, "%d", &(params->data->high_score));
      fclose(f);
    }
  }

  return params;
}

/**
 * \brief Обновление состояния игры в ответ на действие пользователя.
 * \param action Действие пользователя (Start, Pause, Left, Right, Up, Down,
 * Action, Terminate).
 */
void updtInfo(UserAction_t action) {
  GameParams_t *params = getParams();

  if (action == Start) {
    if (*(params->state) == STATE_START) {
      *(params->state) = STATE_GAME;
      clearField(params);
      placeShape(params);
    }
  } else if (action == Pause && *(params->state) != STATE_EXIT) {
    if (*(params->state) == STATE_PAUSE) {
      *(params->state) = STATE_GAME;
      params->data->pause = 0;
    } else {
      *(params->state) = STATE_PAUSE;
      params->data->pause = 1;
    }
  } else if (action == Terminate) {
    freeMemory(params);
  } else if ((action == Left || action == Right) &&
             *(params->state) == STATE_GAME) {
    clearShape(params);
    if (action == Left &&
        params->cur_shape->x - 1 >=
            0 - cntEmptyColsL(params->cur_shape->shape) &&
        isPossbl(params, params->cur_shape->shape, params->cur_shape->x - 1,
                 params->cur_shape->y)) {
      params->cur_shape->x -= 1;
    } else if (action == Right &&
               (params->cur_shape->x + 1) <=
                   6 + cntEmptyColsR(params->cur_shape->shape) &&
               isPossbl(params, params->cur_shape->shape,
                        params->cur_shape->x + 1, params->cur_shape->y)) {
      params->cur_shape->x += 1;
    }
    placeShape(params);
  } else if (action == Down && *(params->state) == STATE_GAME) {
    down(params);
  } else if (action == Action && *(params->state) == STATE_GAME) {
    if (!isSquare(params->cur_shape->shape)) {
      rotate(params);
    }
  } else if (action == Up && *(params->state) == STATE_GAME) {
    autoDown(params);
  }
}