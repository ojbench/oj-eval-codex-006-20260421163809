#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>

// Simple global state for the client
static std::vector<std::string> current_map;

extern int rows;         // The count of rows of the game map.
extern int columns;      // The count of columns of the game map.
extern int total_mines;  // The count of mines of the game map.

// You MUST NOT use any other external variables except for rows, columns and total_mines.

/**
 * @brief The definition of function Execute(int, int, bool)
 *
 * @details This function is designed to take a step when player the client's (or player's) role, and the implementation
 * of it has been finished by TA. (I hope my comments in code would be easy to understand T_T) If you do not understand
 * the contents, please ask TA for help immediately!!!
 *
 * @param r The row coordinate (0-based) of the block to be visited.
 * @param c The column coordinate (0-based) of the block to be visited.
 * @param type The type of operation to a certain block.
 * If type == 0, we'll execute VisitBlock(row, column).
 * If type == 1, we'll execute MarkMine(row, column).
 * If type == 2, we'll execute AutoExplore(row, column).
 * You should not call this function with other type values.
 */
void Execute(int r, int c, int type);

/**
 * @brief The definition of function InitGame()
 *
 * @details This function is designed to initialize the game. It should be called at the beginning of the game, which
 * will read the scale of the game map and the first step taken by the server (see README).
 */
void InitGame() {
  // TODO (student): Initialize all your global variables!
  int first_row, first_column;
  std::cin >> first_row >> first_column;
  current_map.assign(rows, std::string(columns, '?'));
  Execute(first_row, first_column, 0);
}

/**
 * @brief The definition of function ReadMap()
 *
 * @details This function is designed to read the game map from stdin when playing the client's (or player's) role.
 * Since the client (or player) can only get the limited information of the game map, so if there is a 3 * 3 map as
 * above and only the block (2, 0) has been visited, the stdin would be
 *     ???
 *     12?
 *     01?
 */
void ReadMap() {
  // Read the current board into global storage.
  current_map.clear();
  current_map.reserve(rows);
  std::string line;
  for (int i = 0; i < rows; ++i) {
    std::cin >> line;
    current_map.push_back(line);
  }
}

/**
 * @brief The definition of function Decide()
 *
 * @details This function is designed to decide the next step when playing the client's (or player's) role. Open up your
 * mind and make your decision here! Caution: you can only execute once in this function.
 */
void Decide() {
  // Baseline heuristic:
  // 1) If there is a revealed '0' cell, click one of its '?' neighbors (safe expansion).
  // 2) Otherwise click the first '?' on the board.
  auto in_bounds_local = [&](int r, int c) {
    return r >= 0 && r < rows && c >= 0 && c < columns;
  };
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (current_map[i][j] == '0') {
        for (int di = -1; di <= 1; ++di) {
          for (int dj = -1; dj <= 1; ++dj) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di, nj = j + dj;
            if (in_bounds_local(ni, nj) && current_map[ni][nj] == '?') {
              Execute(ni, nj, 0);
              return;
            }
          }
        }
      }
    }
  }
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (current_map[i][j] == '?') {
        Execute(i, j, 0);
        return;
      }
    }
  }
}

#endif
