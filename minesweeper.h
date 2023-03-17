#include <chrono>
#include <initializer_list>
#include <string>
#include <vector>

#ifndef INCLUDE_MINESWEEPER_H_
#define INCLUDE_MINESWEEPER_H_

class Minesweeper {
 public:
  Minesweeper() = default;
  Minesweeper(size_t, size_t, size_t);
  Minesweeper(size_t, size_t,
              const std::initializer_list<std::pair<size_t, size_t>>&);

  ~Minesweeper() = default;

  void NewGame(size_t, size_t, size_t);
  void NewGame(size_t, size_t,
               const std::initializer_list<std::pair<size_t, size_t>>&);

  void OpenCell(size_t, size_t);
  void MarkCell(size_t, size_t);

  std::string GetGameStatus() const;
  double GetGameTime() const;

  std::vector<std::string> RenderField() const;

 private:
  using second_t = std::chrono::duration<double>;
  using clock_t = std::chrono::high_resolution_clock;

  std::vector<std::vector<unsigned char>> field;

  size_t rows;
  size_t cols;

  enum GameStatus { NOT_STARTED, IN_PROGRESS, WON, LOSE };
  GameStatus status;
  std::chrono::time_point<clock_t> start_time;
  std::chrono::time_point<clock_t> end_time;

  size_t cells_to_open;

  std::vector<std::pair<size_t, size_t>> coords{
      {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
};

#endif  // INCLUDE_MINESWEEPER_H_
