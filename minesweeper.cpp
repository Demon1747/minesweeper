#include "minesweeper.h"

#include <queue>
#include <random>
#include <stdexcept>

/// Значения полей во время игры:
/// 0 -- закрытая пустая клетка без мин вокруг
/// от 1 до 8 -- закрытая пустая клетка, указано количество мин вокруг
/// 9 -- закрытая клетка с миной
/// Значения в диапазоне от 10 до 19 -- те же клетки, но открытые (например, 11
/// -- открытая клетка, на которой в обычной игре стояла бы единица)
/// Значения в диапазоне от 20 до 29 -- те же клетки, но с флажком (например, 29
/// -- клетка с миной, на которой поставлен флажок)

/// m -- кол-во строк, n -- столбцов, x -- мин
Minesweeper::Minesweeper(size_t m, size_t n, size_t x)
    : rows(m),
      cols(n),
      status(NOT_STARTED),
      start_time(clock_t::now()),
      end_time(clock_t::now()),
      cells_to_open(m * n - x) {
  /// Создаём поле
  field = std::vector<std::vector<unsigned char>>(m);
  for (size_t i = 0; i < m; ++i) {
    field[i] = std::vector<unsigned char>(n);
  }

  /// Создаем таблицу координат поля для случайной расстановки мин
  std::vector<std::vector<std::pair<size_t, size_t>>> random_field;
  random_field.resize(m);

  for (size_t i = 0; i < rows; ++i) {
    random_field[i].resize(n);
    for (size_t j = 0; j < cols; ++j) {
      random_field[i][j].first = i;
      random_field[i][j].second = j;
    }
  }

  /// Инициализируем ГПСЧ
  std::default_random_engine engine(
      std::chrono::system_clock::now().time_since_epoch().count());

  size_t x_m = 0;
  size_t y_m = 0;

  for (size_t r = 0; r < x; ++r) {
    /// Выбираем случайные значения
    std::uniform_int_distribution<size_t> col_range(0, random_field.size() - 1);
    y_m = col_range(engine);
    std::uniform_int_distribution<size_t> row_range(
        0, random_field[y_m].size() - 1);
    x_m = row_range(engine);

    /// Ставим мину на нужной клетке
    size_t y_f = random_field[y_m][x_m].first;
    size_t x_f = random_field[y_m][x_m].second;
    field[y_f][x_f] = 9;

    /// Удаляем клетку из таблицы для случайной расстановки мин
    random_field[y_m].erase(random_field[y_m].begin() + x_m);
    if (random_field[y_m].empty()) {
      random_field.erase(random_field.begin() + y_m);
    }
  }

  /// Расставляем цифры с количеством мин
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      if (field[i][j] != 9) {
        for (auto shift : coords) {
          /// Проверяем выход за границы поля
          if (i + shift.first >= rows || j + shift.second >= cols) {
            continue;
          } else {
            if (field[i + shift.first][j + shift.second] == 9) {
              field[i][j] += 1;
            }
          }
        }
      }
    }
  }
}

/// m -- кол-во строк, n -- столбцов, init_list -- список координат мин
Minesweeper::Minesweeper(
    size_t m, size_t n,
    const std::initializer_list<std::pair<size_t, size_t>>& init_list)
    : rows(m),
      cols(n),
      status(NOT_STARTED),
      start_time(clock_t::now()),
      end_time(clock_t::now()),
      cells_to_open(m * n - init_list.size()) {
  /// Создаём поле
  field = std::vector<std::vector<unsigned char>>(m);
  for (size_t i = 0; i < rows; ++i) {
    field[i] = std::vector<unsigned char>(n);
  }

  /// Заполняем поле минами
  for (auto elem : init_list) {
    field[elem.first][elem.second] = 9;
  }

  /// Расставляем цифры с количеством мин
  for (size_t i = 0; i < rows; ++i) {
    for (size_t j = 0; j < cols; ++j) {
      if (field[i][j] != 9) {
        for (auto shift : coords) {
          /// Проверяем выход за границы поля
          if (i + shift.first >= rows || j + shift.second >= cols) {
            continue;
          } else {
            if (field[i + shift.first][j + shift.second] == 9) {
              field[i][j] += 1;
            }
          }
        }
      }
    }
  }
}

void Minesweeper::NewGame(size_t m, size_t n, size_t x) {
  *this = Minesweeper(m, n, x);
}

void Minesweeper::NewGame(
    size_t m, size_t n,
    const std::initializer_list<std::pair<size_t, size_t>>& init_list) {
  *this = Minesweeper(m, n, init_list);
}

/// Открывает клетки
void Minesweeper::OpenCell(size_t y, size_t x) {
  std::queue<std::pair<size_t, size_t>> queue;
  switch (status) {
    case NOT_STARTED:
      start_time = clock_t::now();
      status = IN_PROGRESS;
      [[fallthrough]];
    case IN_PROGRESS:
      /// Открытая клетка без мины или флажок
      if (field[y][x] >= 10) {
        return;
      }

      /// Открылась клетка с миной
      if (field[y][x] == 9) {
        status = LOSE;
        end_time = clock_t::now();
        return;
      }

      /// Клетка с цифрой
      if (field[y][x] > 0) {
        field[y][x] += 10;
        cells_to_open -= 1;

        if (cells_to_open == 0) {
          status = WON;
          end_time = clock_t::now();
        }
        return;
      }

      /// У наc нулевая клетка -- открываем набор нулевых клеток с помощью
      /// очереди
      queue.push({y, x});
      field[y][x] = 10;
      cells_to_open -= 1;

      while (!queue.empty()) {
        std::pair<size_t, size_t> cur = queue.front();
        queue.pop();

        /// Осматриваем все клетки вокруг
        for (auto shift : coords) {
          /// При выходе за нулевую границу условие переполнения будет то же
          if (cur.first + shift.first >= rows ||
              cur.second + shift.second >= cols) {
            continue;
          }

          /// В случае нулевой клетки надо открывать все лежащие рядом
          if (field[cur.first + shift.first][cur.second + shift.second] == 0 ||
              field[cur.first + shift.first][cur.second + shift.second] == 20) {
            field[cur.first + shift.first][cur.second + shift.second] = 10;
            queue.push({cur.first + shift.first, cur.second + shift.second});
          } else if (field[cur.first + shift.first][cur.second + shift.second] >
                     20) {
            field[cur.first + shift.first][cur.second + shift.second] -= 10;
          } else if (field[cur.first + shift.first][cur.second + shift.second] <
                     9) {  /// Клетки с цифрами просто открываем
            field[cur.first + shift.first][cur.second + shift.second] += 10;
          } else {
            continue;
          }
          cells_to_open -= 1;
        }
      }

      /// Если все клетки открылись
      if (cells_to_open == 0) {
        status = WON;
        end_time = clock_t::now();
        return;
      }

    case WON:
    case LOSE:
      break;
    default:
      throw std::runtime_error("Invalid status");
  }
}

/// Ставит/убирает флажок
void Minesweeper::MarkCell(size_t y, size_t x) {
  switch (status) {
    case NOT_STARTED:
      start_time = clock_t::now();
      status = IN_PROGRESS;
      [[fallthrough]];
    case IN_PROGRESS:
      if (field[y][x] < 10) {
        /// Ставим флажок
        field[y][x] += 20;
      } else if (field[y][x] >= 20) {
        /// Убираем флажок
        field[y][x] -= 20;
      }

    case WON:
    case LOSE:
      break;
    default:
      throw std::runtime_error("Invalid status");
  }
}

/// Статусы:
/// 0 -- игра не началась
/// 1 -- игра идёт
/// 2 -- игра выиграна
/// 3 -- игра проиграна

/// Возвращает статус игры
std::string Minesweeper::GetGameStatus() const {
  switch (status) {
    case NOT_STARTED:
      return "Not started";
    case IN_PROGRESS:
      return "In progress";
    case WON:
      return "Won";
    case LOSE:
      return "Lose";
    default:
      throw std::runtime_error("Invalid status");
  }
}

/// Возвращает время игры
double Minesweeper::GetGameTime() const {
  switch (status) {
    case NOT_STARTED:
      return 0;
    case IN_PROGRESS:
      return std::chrono::duration_cast<second_t>(clock_t::now() - start_time)
          .count();
    case WON:
    case LOSE:
      return std::chrono::duration_cast<second_t>(end_time - start_time)
          .count();
    default:
      throw std::runtime_error("Invalid status");
  }
}

/// Возвращает vector<string>, представляющий собой поле
std::vector<std::string> Minesweeper::RenderField() const {
  std::vector<std::string> res(rows);
  unsigned char tmp = 0;

  /// Начинаем отрисовывать
  for (size_t i = 0; i < rows; i++) {
    res[i].resize(cols);

    for (size_t j = 0; j < cols; ++j) {
      if ((field[i][j] > 9 && field[i][j] < 20) || status > IN_PROGRESS) {
        /// В случае открытой клетки или законченной игры
        tmp = field[i][j] % 10;
        if (tmp == 0) {
          res[i][j] = '.';
        } else if (tmp < 9) {
          res[i][j] = '0' + tmp;
        } else {
          res[i][j] = '*';
        }
      } else if (field[i][j] < 10) {
        /// В случае закрытой клетки
        res[i][j] = '-';
      } else {
        /// В случае флажка
        res[i][j] = '?';
      }
    }
  }

  return res;
}
