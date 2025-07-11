#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../layer/game.h"
#include "../brick_game/tetris/back.h"

START_TEST(back_setNewShape) {
  int **shape;
  int shapes_ref[7][PIECE_SIZE][PIECE_SIZE] = {
      {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}};

  shape = calloc(PIECE_SIZE, sizeof(int *));
  ck_assert_ptr_nonnull(shape);

  for (int i = 0; i < PIECE_SIZE; i++) {
    shape[i] = calloc(PIECE_SIZE, sizeof(int));
    ck_assert_ptr_nonnull(shape[i]);
  }

  setNewShape(shape);

  int eq = 1;
  int matched = 0;
  for (int s = 0; s < 7 && !matched; s++) {
    for (int i = 0; i < PIECE_SIZE && eq; i++) {
      for (int j = 0; j < PIECE_SIZE && eq; j++) {
        if (shape[i][j] != shapes_ref[s][i][j]) {
          eq = 0;
        }
      }
    }
    if (eq) {
      matched = 1;
    } else {
      eq = 1;
    }
  }
  ck_assert_msg(matched, "setNewShape produced an unexpected shape pattern");

  for (int i = 0; i < PIECE_SIZE; i++) {
    free(shape[i]);
  }
  free(shape);
}
END_TEST

START_TEST(back_setStat) {
  GameParams_t params;
  GameInfo_t data;
  params.data = &data;

  params.data->score = 123;
  params.data->high_score = 456;
  params.data->level = 7;
  params.data->speed = 2000;
  params.data->pause = 2;

  setStat(&params);

  ck_assert_int_eq(params.data->score, 0);
  ck_assert_int_eq(params.data->high_score, 0);
  ck_assert_int_eq(params.data->level, 1);
  ck_assert_int_eq(params.data->speed, 1000);
  ck_assert_int_eq(params.data->pause, 0);
}
END_TEST

START_TEST(back_setCurShape) {
  GameParams_t *p = malloc(sizeof *p);
  ck_assert_ptr_nonnull(p);

  setCurShape(p);

  ck_assert_ptr_nonnull(p->cur_shape);
  ck_assert_ptr_nonnull(p->cur_shape->shape);
  for (int i = 0; i < PIECE_SIZE; i++) {
    ck_assert_ptr_nonnull(p->cur_shape->shape[i]);
  }

  ck_assert_int_eq(p->cur_shape->x, 3);
  ck_assert_int_eq(p->cur_shape->y, 0);
  ck_assert_int_ge(p->cur_shape->color, 1);
  ck_assert_int_le(p->cur_shape->color, 7);

  freeMemory(p);
}
END_TEST

START_TEST(back_getParams) {
  const int expected_high = 123;
  FILE *f = fopen("record.txt", "w");
  if (f == NULL) {
    ck_abort_msg("Failed to create record.txt");
  }
  fprintf(f, "%d\n", expected_high);
  fclose(f);

  GameParams_t *p1 = getParams();
  ck_assert_ptr_nonnull(p1);

  GameParams_t *p2 = getParams();
  ck_assert_ptr_eq(p1, p2);

  ck_assert_int_eq(*(p1->state), STATE_START);
  ck_assert_int_eq(p1->data->score, 0);
  ck_assert_int_eq(p1->data->level, 1);
  ck_assert_int_eq(p1->data->speed, 1000);
  ck_assert_int_eq(p1->data->pause, 0);

  ck_assert_int_eq(p1->data->high_score, expected_high);

  ck_assert_int_eq(p1->cur_shape->x, 3);
  ck_assert_int_eq(p1->cur_shape->y, 0);
  ck_assert_int_ge(p1->cur_shape->color, 1);
  ck_assert_int_le(p1->cur_shape->color, 7);

  int rm = remove("record.txt");
  ck_assert_msg(rm == 0, "Failed to remove record.txt");

  freeMemory(p1);
}
END_TEST

START_TEST(back_clearField) {
  GameParams_t *params = getParams();

  clearField(params);

  for (int i = 0; i < FIELD_HEIGHT; ++i) {
    for (int j = 0; j < FIELD_WIDTH; ++j) {
      ck_assert_int_eq(params->data->field[i][j], 0);
    }
  }

  freeMemory(params);
}
END_TEST

START_TEST(back_clearShape) {
  GameParams_t *params = getParams();

  for (int i = 0; i < FIELD_HEIGHT; i++) {
    for (int j = 0; j < FIELD_WIDTH; j++) {
      params->data->field[i][j] = 9;
    }
  }

  int shape_[4][4] = {{1, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}};

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      params->cur_shape->shape[i][j] = shape_[i][j];
    }
  }
  params->cur_shape->x = 3;
  params->cur_shape->y = 5;

  clearShape(params);

  for (int i = 0; i < FIELD_HEIGHT; i++) {
    for (int j = 0; j < FIELD_WIDTH; j++) {
      if ((j == 0 + 3 && i <= 5 + 2 && i >= 5 + 0) ||
          (j == 1 + 3 && i == 5 + 2)) {
        ck_assert_int_eq(params->data->field[i][j], 0);
      } else {
        ck_assert_int_eq(params->data->field[i][j], 9);
      }
    }
  }

  freeMemory(params);
}
END_TEST

START_TEST(back_placeShape) {
  GameParams_t *params = getParams();
  int shape_[4][4] = {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      params->cur_shape->shape[i][j] = shape_[i][j];
    }
  }
  params->cur_shape->x = 4;
  params->cur_shape->y = 6;
  params->cur_shape->color = 5;

  clearField(params);
  placeShape(params);

  for (int i = 0; i < FIELD_HEIGHT; i++) {
    for (int j = 0; j < FIELD_WIDTH; j++) {
      if (i >= 6 && i <= 7 && j >= 4 && j <= 5) {
        ck_assert_int_eq(params->data->field[i][j], 5);
      } else {
        ck_assert_int_eq(params->data->field[i][j], 0);
      }
    }
  }

  freeMemory(params);
}
END_TEST

START_TEST(back_cntEmptyColsL) {
  GameParams_t *params = getParams();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      params->cur_shape->shape[i][j] = 0;
    }
  }

  ck_assert_int_eq(cntEmptyColsL(params->cur_shape->shape), PIECE_SIZE);

  params->cur_shape->shape[1][0] = 1;
  ck_assert_int_eq(cntEmptyColsL(params->cur_shape->shape), 0);

  params->cur_shape->shape[1][0] = 0;
  params->cur_shape->shape[0][1] = 2;
  ck_assert_int_eq(cntEmptyColsL(params->cur_shape->shape), 1);

  params->cur_shape->shape[0][1] = 0;
  params->cur_shape->shape[3][2] = 3;
  ck_assert_int_eq(cntEmptyColsL(params->cur_shape->shape), 2);

  freeMemory(params);
}
END_TEST

START_TEST(back_cntEmptyColsR) {
  GameParams_t *params = getParams();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      params->cur_shape->shape[i][j] = 0;
    }
  }

  ck_assert_int_eq(cntEmptyColsR(params->cur_shape->shape), PIECE_SIZE);

  params->cur_shape->shape[2][3] = 1;
  ck_assert_int_eq(cntEmptyColsR(params->cur_shape->shape), 0);

  params->cur_shape->shape[2][3] = 0;
  params->cur_shape->shape[0][2] = 2;
  ck_assert_int_eq(cntEmptyColsR(params->cur_shape->shape), 1);

  params->cur_shape->shape[0][2] = 0;
  params->cur_shape->shape[3][1] = 3;
  ck_assert_int_eq(cntEmptyColsR(params->cur_shape->shape), 2);

  freeMemory(params);
}
END_TEST

START_TEST(back_height) {
  GameParams_t *params = getParams();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      params->cur_shape->shape[i][j] = 0;
    }
  }

  ck_assert_int_eq(height(params->cur_shape->shape), 0);

  params->cur_shape->shape[0][0] = 1;
  ck_assert_int_eq(height(params->cur_shape->shape), 1);

  params->cur_shape->shape[1][1] = 2;
  ck_assert_int_eq(height(params->cur_shape->shape), 2);

  params->cur_shape->shape[0][0] = 0;
  params->cur_shape->shape[1][1] = 0;
  params->cur_shape->shape[PIECE_SIZE - 1][0] = 3;
  ck_assert_int_eq(height(params->cur_shape->shape), 1);

  params->cur_shape->shape[PIECE_SIZE - 1][0] = 0;
  params->cur_shape->shape[1][1] = 1;
  params->cur_shape->shape[2][2] = 1;
  params->cur_shape->shape[3][3] = 1;
  ck_assert_int_eq(height(params->cur_shape->shape), 3);

  params->cur_shape->shape[0][0] = 1;
  ck_assert_int_eq(height(params->cur_shape->shape), 4);

  freeMemory(params);
}
END_TEST

START_TEST(back_isPossbl) {
  GameParams_t *params = getParams();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      params->cur_shape->shape[i][j] = 0;
    }
  }

  params->cur_shape->shape[0][0] = 1;
  params->cur_shape->x = 3;
  params->cur_shape->y = 5;

  ck_assert_int_eq(isPossbl(params, params->cur_shape->shape,
                            params->cur_shape->x, params->cur_shape->y),
                   1);
  ck_assert_int_eq(isPossbl(params, params->cur_shape->shape,
                            params->cur_shape->x, FIELD_HEIGHT),
                   0);
  ck_assert_int_eq(isPossbl(params, params->cur_shape->shape, -4, 0), 0);
  ck_assert_int_eq(isPossbl(params, params->cur_shape->shape, FIELD_WIDTH, 0),
                   0);

  params->cur_shape->shape[0][0] = params->cur_shape->shape[0][1] =
      params->cur_shape->shape[1][0] = params->cur_shape->shape[1][1] = 1;
  params->data->field[3][2] = 9;
  ck_assert_int_eq(isPossbl(params, params->cur_shape->shape, 2, 3), 0);

  freeMemory(params);
}
END_TEST

START_TEST(back_isSquare) {
  GameParams_t *params = getParams();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      params->cur_shape->shape[i][j] = 0;
    }
  }

  params->cur_shape->shape[0][1] = 1;
  params->cur_shape->shape[0][2] = 1;
  params->cur_shape->shape[1][1] = 1;
  params->cur_shape->shape[1][2] = 1;
  ck_assert_int_eq(isSquare(params->cur_shape->shape), 1);

  params->cur_shape->shape[1][2] = 0;
  ck_assert_int_eq(isSquare(params->cur_shape->shape), 0);

  params->cur_shape->shape[0][1] = 0;
  params->cur_shape->shape[0][2] = 0;
  params->cur_shape->shape[1][1] = params->cur_shape->shape[1][2] =
      params->cur_shape->shape[2][1] = params->cur_shape->shape[2][2] = 1;
  ck_assert_int_eq(isSquare(params->cur_shape->shape), 0);

  freeMemory(params);
}
END_TEST

START_TEST(back_isVertical) {
  int shape[PIECE_SIZE][PIECE_SIZE];
  for (int i = 0; i < PIECE_SIZE; i++) {
    for (int j = 0; j < PIECE_SIZE; j++) {
      if (j == 0) {
        shape[i][j] = 1;
      } else {
        shape[i][j] = 0;
      }
    }
  }

  ck_assert_int_eq(isVertical(shape), 1);

  for (int i = 0; i < PIECE_SIZE; i++) {
    for (int j = 0; j < PIECE_SIZE; j++) {
      shape[i][j] = 0;
    }
  }
  for (int j = 0; j < PIECE_SIZE; j++) {
    shape[0][j] = 1;
  }
  ck_assert_int_eq(isVertical(shape), 0);

  for (int i = 0; i < PIECE_SIZE; i++) {
    for (int j = 0; j < PIECE_SIZE; j++) {
      shape[i][j] = 0;
    }
  }
  shape[0][0] = shape[1][0] = 1;
  shape[2][1] = shape[3][1] = 1;
  ck_assert_int_eq(isVertical(shape), 0);
}
END_TEST

START_TEST(back_rotate) {
  GameParams_t *params = getParams();

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (i == 0) {
        params->cur_shape->shape[i][j] = 1;
      } else {
        params->cur_shape->shape[i][j] = 0;
      }
    }
  }

  params->cur_shape->x = 0;
  params->cur_shape->y = 0;
  params->cur_shape->color = 1;

  placeShape(params);
  rotate(params);

  for (int i = 0; i < PIECE_SIZE; i++) {
    for (int j = 0; j < PIECE_SIZE; j++) {
      int expected_cell = (j == 1) ? 1 : 0;
      ck_assert_int_eq(params->cur_shape->shape[i][j], expected_cell);
      ck_assert_int_eq(params->data->field[params->cur_shape->y + i]
                                          [params->cur_shape->x + j],
                       expected_cell * params->cur_shape->color);
    }
  }

  freeMemory(params);
}
END_TEST

START_TEST(back_hasCollisBellow) {
  GameParams_t *p = getParams();
  clearField(p);

  ck_assert_int_eq(hasCollisBellow(p), 0);

  p->data->field[1][4] = 1;
  p->data->field[2][4] = 1;
  ck_assert_int_eq(hasCollisBellow(p), 1);

  p->cur_shape->y = FIELD_HEIGHT - 1;
  ck_assert_int_eq(hasCollisBellow(p), 1);

  freeMemory(p);
}
END_TEST

START_TEST(back_spawnNew) {
  GameParams_t *p = getParams();
  clearField(p);
  p->cur_shape->x = 5;
  p->cur_shape->y = 5;

  int testNext[PIECE_SIZE][PIECE_SIZE] = {
      {1, 1, 1, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}};
  for (int i = 0; i < PIECE_SIZE; ++i)
    for (int j = 0; j < PIECE_SIZE; ++j) p->data->next[i][j] = testNext[i][j];

  spawnNew(p);

  for (int i = 0; i < PIECE_SIZE; ++i) {
    for (int j = 0; j < PIECE_SIZE; ++j) {
      ck_assert_int_eq(p->cur_shape->shape[i][j], testNext[i][j]);
    }
  }

  ck_assert_int_eq(p->cur_shape->x, 3);
  ck_assert_int_eq(p->cur_shape->y, 0);

  ck_assert_int_ge(p->cur_shape->color, 1);
  ck_assert_int_le(p->cur_shape->color, 7);

  int cnt = 0;
  for (int i = 0; i < PIECE_SIZE; ++i)
    for (int j = 0; j < PIECE_SIZE; ++j)
      if (p->data->next[i][j] == 1) ++cnt;
  ck_assert_int_eq(cnt, 4);

  freeMemory(p);
}
END_TEST

START_TEST(back_updtScore) {
  GameParams_t *p = getParams();

  ck_assert_int_eq(p->data->score, 0);

  updtScore(p, 1);
  ck_assert_int_eq(p->data->score, 100);

  updtScore(p, 2);
  ck_assert_int_eq(p->data->score, 400);

  updtScore(p, 3);
  ck_assert_int_eq(p->data->score, 1100);

  updtScore(p, 4);
  ck_assert_int_eq(p->data->score, 2600);

  updtScore(p, 5);
  ck_assert_int_eq(p->data->score, 4100);

  freeMemory(p);
}
END_TEST

START_TEST(back_updtHighScore) {
  const int initial_high = 50;
  FILE *f = fopen("record.txt", "w");
  ck_assert_ptr_nonnull(f);
  fprintf(f, "%d\n", initial_high);
  fclose(f);

  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);

  p->data->score = initial_high - 10;
  updtHighScore(p);
  ck_assert_int_eq(p->data->high_score, initial_high);

  f = fopen("record.txt", "r");
  ck_assert_ptr_nonnull(f);
  int file_val;
  fscanf(f, "%d", &file_val);
  fclose(f);
  ck_assert_int_eq(file_val, initial_high);

  p->data->score = initial_high + 20;
  updtHighScore(p);
  ck_assert_int_eq(p->data->high_score, initial_high + 20);
  f = fopen("record.txt", "r");
  ck_assert_ptr_nonnull(f);
  fscanf(f, "%d", &file_val);
  fclose(f);
  ck_assert_int_eq(file_val, p->data->high_score);

  freeMemory(p);

  int rm = remove("record.txt");
  ck_assert_msg(rm == 0, "Failed to remove record.txt");
}
END_TEST

START_TEST(back_updtLevel) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);

  int new_lev = 600;
  p->data->score = 500;
  updtLevel(p, &new_lev, 1);
  ck_assert_int_eq(p->data->level, 1);
  ck_assert_int_eq(new_lev, 600);
  ck_assert_int_eq(p->data->speed, 1000);

  p->data->score = new_lev;
  updtLevel(p, &new_lev, 0);
  ck_assert_int_eq(p->data->level, 1);
  ck_assert_int_eq(new_lev, 600);
  ck_assert_int_eq(p->data->speed, 1000);

  int old_speed = p->data->speed;
  updtLevel(p, &new_lev, 1);
  ck_assert_int_eq(p->data->level, 2);
  ck_assert_int_eq(new_lev, 600 + 600);
  ck_assert_int_eq(p->data->speed, old_speed - 100);

  p->data->level = 10;
  int curr_new_lev = new_lev;
  int curr_speed = p->data->speed;
  p->data->score = curr_new_lev + 1000;
  updtLevel(p, &new_lev, 1);
  ck_assert_int_eq(p->data->level, 10);
  ck_assert_int_eq(new_lev, curr_new_lev);
  ck_assert_int_eq(p->data->speed, curr_speed);

  freeMemory(p);
}
END_TEST

START_TEST(back_checkLines) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);
  clearField(p);

  checkLines(p);
  ck_assert_int_eq(p->data->score, 0);

  int last = FIELD_HEIGHT - 1;
  for (int x = 0; x < FIELD_WIDTH; ++x) {
    p->data->field[last][x] = 1;
  }

  p->data->field[last - 1][0] = 2;

  checkLines(p);

  ck_assert_int_eq(p->data->field[last][0], 2);
  for (int x = 1; x < FIELD_WIDTH; ++x) {
    ck_assert_int_eq(p->data->field[last][x], 0);
  }
  ck_assert_int_eq(p->data->score, 100);
  ck_assert_int_eq(p->data->high_score, 100);

  FILE *f = fopen("record.txt", "r");
  ck_assert_msg(f != NULL, "record.txt was not created");
  int file_score = 0;
  fscanf(f, "%d", &file_score);
  fclose(f);
  ck_assert_int_eq(file_score, 100);

  remove("record.txt");
  freeMemory(p);
}
END_TEST

START_TEST(back_autoDown_no_collision) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);
  clearField(p);

  int old_y = p->cur_shape->y;

  autoDown(p);

  ck_assert_int_eq(p->cur_shape->y, old_y + 1);
  ck_assert_int_eq(*(p->state), STATE_START);
  ck_assert_int_eq(p->data->pause, 0);

  freeMemory(p);
}
END_TEST

START_TEST(back_autoDown_collision) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);
  clearField(p);

  p->cur_shape->y = FIELD_HEIGHT - height(p->cur_shape->shape);

  autoDown(p);

  ck_assert_int_eq(p->cur_shape->y, 0);
  ck_assert_int_eq(*(p->state), STATE_START);

  freeMemory(p);
}
END_TEST

START_TEST(back_down) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);
  clearField(p);
  p->cur_shape->color = 1;
  for (int i = 0; i < PIECE_SIZE; ++i)
    for (int j = 0; j < PIECE_SIZE; ++j)
      if (i == 0) {
        p->cur_shape->shape[i][j] = 1;
      } else {
        p->cur_shape->shape[i][j] = 0;
      }

  p->cur_shape->x = 4;
  p->cur_shape->y = 4;

  down(p);
  for (int i = 4; i < FIELD_HEIGHT; ++i)
    for (int j = 0; j < FIELD_WIDTH; ++j)
      if (i == FIELD_HEIGHT - 1 && j >= 4 && j <= 7) {
        ck_assert_int_eq(p->data->field[i][j], 1);
      } else {
        ck_assert_int_eq(p->data->field[i][j], 0);
      }

  ck_assert_int_eq(p->cur_shape->y, 0);
  ck_assert_int_eq(*(p->state), STATE_START);
  ck_assert_int_eq(p->data->pause, 0);
  ck_assert_int_eq(p->data->score, 0);

  freeMemory(p);
}
END_TEST

START_TEST(back_updtInfo) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);

  ck_assert_int_eq(*(p->state), STATE_START);

  p->data->field[5][5] = 52;
  updtInfo(Start);
  ck_assert_int_eq(*(p->state), STATE_GAME);
  ck_assert_int_eq(p->data->field[5][5], 0);

  /* test Left/Right movement */
  int x0 = p->cur_shape->x;
  updtInfo(Left);
  ck_assert_int_eq(*(p->state), STATE_GAME);
  ck_assert_int_eq(p->cur_shape->x, x0 - 1);
  updtInfo(Right);
  ck_assert_int_eq(p->cur_shape->x, x0);

  /* test Pause toggling */
  updtInfo(Pause);
  ck_assert_int_eq(*(p->state), STATE_PAUSE);
  ck_assert_int_eq(p->data->pause, 1);
  updtInfo(Pause);
  ck_assert_int_eq(*(p->state), STATE_GAME);
  ck_assert_int_eq(p->data->pause, 0);

  updtInfo(Down);
  ck_assert_int_eq(p->cur_shape->y, 0);

  /* terminate frees everything */
  updtInfo(Terminate);
}
END_TEST

START_TEST(layer_userInput) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);
  ck_assert_int_eq(*(p->state), STATE_START);

  userInput(Start, true);
  ck_assert_int_eq(*(p->state), STATE_GAME);

  userInput(Start, false);
  ck_assert_int_eq(*(p->state), STATE_GAME);

  userInput(Pause, true);
  ck_assert_int_eq(*(p->state), STATE_PAUSE);
  ck_assert_int_eq(p->data->pause, 1);

  userInput(Pause, false);
  ck_assert_int_eq(*(p->state), STATE_GAME);
  ck_assert_int_eq(p->data->pause, 0);

  userInput(Terminate, false);
}
END_TEST

START_TEST(layer_updateCurrentState) {
  GameParams_t *p = getParams();
  ck_assert_ptr_nonnull(p);

  p->data->score = 10;
  p->data->high_score = 20;
  p->data->level = 3;
  p->data->speed = 777;
  p->data->pause = 9;
  p->data->field[0][0] = 123;
  p->data->next[1][1] = 55;

  GameInfo_t gi = updateCurrentState();

  ck_assert_int_eq(gi.score, 10);
  ck_assert_int_eq(gi.high_score, 20);
  ck_assert_int_eq(gi.level, 3);
  ck_assert_int_eq(gi.speed, 777);
  ck_assert_int_eq(gi.pause, 9);

  ck_assert_ptr_eq(gi.field, p->data->field);
  ck_assert_ptr_eq(gi.next, p->data->next);

  ck_assert_int_eq(gi.field[0][0], 123);
  ck_assert_int_eq(gi.next[1][1], 55);

  freeMemory(p);
}
END_TEST

static Suite *tetris_suite(void) {
  Suite *s = suite_create("tetris");
  TCase *tc_core = tcase_create("Core");

  tcase_add_test(tc_core, back_setNewShape);
  tcase_add_test(tc_core, back_setStat);
  tcase_add_test(tc_core, back_setCurShape);
  tcase_add_test(tc_core, back_getParams);
  tcase_add_test(tc_core, back_clearField);
  tcase_add_test(tc_core, back_clearShape);
  tcase_add_test(tc_core, back_placeShape);
  tcase_add_test(tc_core, back_cntEmptyColsL);
  tcase_add_test(tc_core, back_cntEmptyColsR);
  tcase_add_test(tc_core, back_height);
  tcase_add_test(tc_core, back_isPossbl);
  tcase_add_test(tc_core, back_isSquare);
  tcase_add_test(tc_core, back_isVertical);
  tcase_add_test(tc_core, back_rotate);
  tcase_add_test(tc_core, back_hasCollisBellow);
  tcase_add_test(tc_core, back_spawnNew);
  tcase_add_test(tc_core, back_updtScore);
  tcase_add_test(tc_core, back_updtHighScore);
  tcase_add_test(tc_core, back_updtLevel);
  tcase_add_test(tc_core, back_checkLines);
  tcase_add_test(tc_core, back_autoDown_no_collision);
  tcase_add_test(tc_core, back_autoDown_collision);
  tcase_add_test(tc_core, back_down);
  tcase_add_test(tc_core, back_updtInfo);

  tcase_add_test(tc_core, layer_userInput);
  tcase_add_test(tc_core, layer_updateCurrentState);

  suite_add_tcase(s, tc_core);
  return s;
}

int main(void) {
  int number_failed;
  Suite *s = tetris_suite();
  SRunner *sr = srunner_create(s);

  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}