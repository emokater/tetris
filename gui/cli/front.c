/**
 * \file front.c
 * \brief Интерфейсная часть игры Tetris на ncurses: отрисовка, обработка ввода
 * и запуск цикла.
 */

#include <locale.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../../layer/game.h"

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define DELAY 50

/**
 * \brief Инициализация библиотеки ncurses и цветовых пар для вывода.
 *
 * Настраивает локаль для поддержки Unicode, отключает отображение
 * вводимых символов и курсора, включает чтение специальных клавиш и
 * настраивает неблокирующий ввод. Также определяет цветовые пары для фигур и
 * игрового окна.
 */
static void startNcurses() {
  setlocale(LC_ALL, "");
  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);

  start_color();
  init_color(8, 1000, 400, 700);

  init_pair(1, COLOR_BLUE, COLOR_BLUE);
  init_pair(2, COLOR_CYAN, COLOR_CYAN);
  init_pair(3, COLOR_GREEN, COLOR_GREEN);
  init_pair(4, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(5, COLOR_RED, COLOR_RED);
  init_pair(6, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(7, COLOR_WHITE, COLOR_WHITE);
  init_pair(8, 8, COLOR_BLACK);
  init_pair(9, COLOR_BLACK, 8);
  init_pair(10, 8, 8);

  clear();
  refresh();
}

/**
 * \brief Завершение работы с ncurses и удаление окон.
 * Освобождает память, занятую окнами, и завершает режим ncurses.
 *
 * \param w1 Игровое окно.
 * \param w2 Окно статистики.
 * \param w3 Окно следующей фигуры.
 */
static void endNcurses(WINDOW *w1, WINDOW *w2, WINDOW *w3) {
  delwin(w1);
  delwin(w2);
  delwin(w3);
  endwin();
}

/**
 * @brief Оформляет ncurses-окна.
 *
 * Устанавливает цветовую пару для рисования рамок, рисует рамки вокруг
 * каждого окна и добавляет заголовки «GAME», «STAT» и «NEXT».
 *
 * @param gaming     Окно игрового поля (размер FIELD_HEIGHT+2 ×
 * 2*FIELD_WIDTH+2).
 * @param statistics Окно статистики (счет, рекорд, уровень) (размер 8 ×
 * 2*FIELD_WIDTH).
 * @param next       Окно превью следующей фигуры (размер 8 × 2*FIELD_WIDTH).
 */
static void setWindows(WINDOW *gaming, WINDOW *statistics, WINDOW *next) {
  wattron(gaming, COLOR_PAIR(8));
  wattron(statistics, COLOR_PAIR(8));
  wattron(next, COLOR_PAIR(8));

  box(gaming, ACS_VLINE, ACS_HLINE);
  box(statistics, ACS_VLINE, ACS_HLINE);
  box(next, ACS_VLINE, ACS_HLINE);

  mvwprintw(next, 0, FIELD_WIDTH - 2, "NEXT");
  mvwprintw(gaming, 0, 9, "GAME");
  mvwprintw(statistics, 0, FIELD_WIDTH - 2, "STAT");

  wattroff(gaming, COLOR_PAIR(8));
  wattroff(statistics, COLOR_PAIR(8));
  wattroff(next, COLOR_PAIR(8));
}

/**
 * \brief Преобразует код введённой клавиши в действие пользователя.
 * \param input Код клавиши, полученный от getch().
 * \return Действие пользователя типа UserAction_t.
 */
static UserAction_t actionProcessing(int input) {
  UserAction_t res;

  if (input == ' ' || input == '\n' || input == KEY_ENTER) {
    res = Start;
  } else if (input == 'a' || input == 'A') {
    res = Left;
  } else if (input == 'd' || input == 'D') {
    res = Right;
  } else if (input == 's' || input == 'S') {
    res = Down;
  } else if (input == 'r' || input == 'R') {
    res = Action;
  } else if (input == 'p' || input == 'P') {
    res = Pause;
  } else if (input == 'c' || input == 'C') {
    res = Terminate;
  } else {
    res = Up;
  }

  return res;
}

/**
 * \brief Отрисовывает игровое поле в заданном окне.
 *
 * Если игра не на паузе, очищает область и рисует все клетки фигур
 * с учётом их цвета. При состоянии паузы и конце игры рисует специальные
 * индикаторы.
 *
 * \param win Окно, в котором рисуется игровое поле.
 * \param info Указатель на структуру GameInfo_t с текущими данными игры.
 */
static void drawField(WINDOW *win, const GameInfo_t *info) {
  if (info->pause == 0) {
    for (int y = 0; y < FIELD_HEIGHT; ++y) {
      for (int x = 0; x < 2 * FIELD_WIDTH; ++x) {
        mvwaddch(win, y + 1, x + 1, ' ');
      }
    }

    for (int y = 0; y < FIELD_HEIGHT; ++y) {
      for (int x = 0; x < FIELD_WIDTH; ++x) {
        int c = info->field[y][x];
        if (c) {
          wattron(win, COLOR_PAIR(c));
          mvwaddch(win, y + 1, 2 * x + 1, ' ');
          mvwaddch(win, y + 1, 2 * x + 2, ' ');
          wattroff(win, COLOR_PAIR(c));
        }
      }
    }
  }

  if (info->pause == 1) {
    int pause[5][3] = {
        {10, 0, 0}, {10, 10, 0}, {10, 10, 10}, {10, 10, 0}, {10, 0, 0}};
    for (int y = 0; y < 5; ++y) {
      for (int x = 0; x < 3; ++x) {
        int c = pause[y][x];
        if (c != 0) {
          wattron(win, COLOR_PAIR(c));
          mvwaddch(win, y + 8, 2 * x + 8, ' ');
          mvwaddch(win, y + 8, 2 * x + 9, ' ');
          wattroff(win, COLOR_PAIR(c));
        }
      }
    }
  }

  if (info->pause == 2) {
    wattron(win, COLOR_PAIR(9));
    mvwprintw(win, 10, 6, "GAME OVER");
    wattron(win, COLOR_PAIR(9));
  }
}

/**
 * \brief Отрисовывает панель статистики (счёт, рекорд, уровень).
 * \param win Окно статистики.
 * \param info Указатель на структуру GameInfo_t с текущими данными игры.
 */
static void drawStat(WINDOW *win, const GameInfo_t *info) {
  for (int y = 0; y < 5; ++y) {
    for (int x = 0; x < 2 * FIELD_WIDTH - 2; ++x) {
      mvwaddch(win, y + 1, x + 1, ' ');
    }
  }

  wattron(win, COLOR_PAIR(8));
  mvwprintw(win, 2, 2, "Score:      %d", info->score);
  mvwprintw(win, 3, 2, "High Score: %d", info->high_score);
  mvwprintw(win, 5, 2, "Level:      %d", info->level);
  wattroff(win, COLOR_PAIR(8));
}

/**
 * \brief Отрисовывает окно с превью следующей фигуры.
 * \param win Окно превью следующей фигуры.
 * \param info Указатель на структуру GameInfo_t с текущими данными игры.
 */
static void drawNext(WINDOW *win, const GameInfo_t *info) {
  for (int y = 1; y < 7; ++y) {
    for (int x = 1; x < 2 * FIELD_WIDTH - 1; ++x) {
      mvwaddch(win, y, x, ' ');
    }
  }

  for (int y = 0; y < 4; ++y) {
    for (int x = 0; x < 4; ++x) {
      int c = info->next[y][x];
      if (c) {
        wattron(win, COLOR_PAIR(c));
        mvwaddch(win, y + 3, 2 * x + 6, ' ');
        mvwaddch(win, y + 3, 2 * x + 7, ' ');
        wattroff(win, COLOR_PAIR(c));
      }
    }
  }
}

/**
 * \brief Обновляет все окна на экране – игровое поле, статистику, превью.
 * \param tmpGS Структура GameInfo_t с текущим состоянием игры.
 * \param next Окно для превью следующей фигуры.
 * \param gaming Игровое окно.
 * \param statistics Окно статистики.
 */
static void updtScreen(GameInfo_t tmpGS, WINDOW *next, WINDOW *gaming,
                       WINDOW *statistics) {
  drawNext(next, &tmpGS);
  drawField(gaming, &tmpGS);
  drawStat(statistics, &tmpGS);

  wrefresh(next);
  wrefresh(gaming);
  wrefresh(statistics);
}

/**
 * \brief Главная функция: инициализирует ncurses, запускает игровой цикл
 * Tetris.
 *
 * Инициализирует интерфейс, создаёт окна, обрабатывает ввод и таймер,
 * обновляет экран до завершения игры, затем завершает работу ncurses.
 *
 * \return Код возврата (0 при успешном завершении).
 */
int main() {
  srand((unsigned)time(NULL));
  startNcurses();

  WINDOW *gaming = newwin(FIELD_HEIGHT + 2, 2 * FIELD_WIDTH + 2, 0, 0);
  WINDOW *statistics = newwin(8, 2 * FIELD_WIDTH, 0, 2 * FIELD_WIDTH + 2);
  WINDOW *next = newwin(8, 2 * FIELD_WIDTH, 8, 2 * FIELD_WIDTH + 2);
  setWindows(gaming, statistics, next);

  int game = 1;
  int ms_storage = 0;
  UserAction_t act = Start;
  userInput(act, false);
  GameInfo_t tmpGS = updateCurrentState();
  updtScreen(tmpGS, next, gaming, statistics);

  while (game) {
    int sig = getch();
    act = actionProcessing(sig);

    if (act == Terminate) {
      userInput(act, false);
      game = 0;
    } else {
      if (act != Up) {
        userInput(act, false);
        tmpGS = updateCurrentState();
      }

      ms_storage += DELAY;
      if (ms_storage > tmpGS.speed) {
        userInput(Up, false);
        ms_storage = 0;
        tmpGS = updateCurrentState();
      }

      updtScreen(tmpGS, next, gaming, statistics);
    }
    napms(DELAY);
  }

  endNcurses(gaming, statistics, next);

  return 0;
}