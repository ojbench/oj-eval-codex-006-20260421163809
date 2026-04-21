#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include <limits>

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
  // Baseline1-style heuristic (single action per call):
  // A) If a number cell has (k - marked) == unknown and unknown>0, mark one unknown neighbor as a mine.
  // B) If a number cell has marked == k and unknown>0, auto-explore around it.
  // C) Else if there is a revealed '0', click one of its '?' neighbors.
  // D) Else click the first '?'.
  auto in_bounds_local = [&](int r, int c) { return r >= 0 && r < rows && c >= 0 && c < columns; };

  // Gather candidates for A and B
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      char ch = current_map[i][j];
      if (ch >= '0' && ch <= '8') {
        int k = ch - '0';
        int unknown = 0, marked = 0;
        int first_unknown_r = -1, first_unknown_c = -1;
        for (int di = -1; di <= 1; ++di) {
          for (int dj = -1; dj <= 1; ++dj) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di, nj = j + dj;
            if (!in_bounds_local(ni, nj)) continue;
            if (current_map[ni][nj] == '?') {
              if (first_unknown_r == -1) { first_unknown_r = ni; first_unknown_c = nj; }
              ++unknown;
            } else if (current_map[ni][nj] == '@') {
              ++marked;
            }
          }
        }
        if (unknown > 0) {
          if (k - marked == unknown) {
            // All unknown neighbors are mines: mark one
            Execute(first_unknown_r, first_unknown_c, 1);
            return;
          }
          if (marked == k) {
            // All unknown neighbors are safe: auto-explore this number cell
            Execute(i, j, 2);
            return;
          }
        }
      }
    }
  }

  // C) Expand near zeros
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

  // D) Smarter fallback: choose a '?' adjacent to numbers, preferring near '0's, else minimal risk proxy.
  int cand_r = -1, cand_c = -1;
  // D1: any '?' adjacent to a zero
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (current_map[i][j] != '?') continue;
      bool near_zero = false;
      for (int di = -1; di <= 1 && !near_zero; ++di) {
        for (int dj = -1; dj <= 1 && !near_zero; ++dj) {
          if (di == 0 && dj == 0) continue;
          int ni = i + di, nj = j + dj;
          if (in_bounds_local(ni, nj) && current_map[ni][nj] == '0') near_zero = true;
        }
      }
      if (near_zero) { cand_r = i; cand_c = j; goto FALLBACK_CHOSEN; }
    }
  }
  // D2: minimize simple risk proxy accumulated from adjacent numbers
  {
    double best_score = std::numeric_limits<double>::infinity();
    for (int i = 0; i < rows; ++i) {
      for (int j = 0; j < columns; ++j) {
        if (current_map[i][j] != '?') continue;
        bool has_num_neighbor = false;
        double score = 0.0;
        for (int di = -1; di <= 1; ++di) {
          for (int dj = -1; dj <= 1; ++dj) {
            if (di == 0 && dj == 0) continue;
            int ni = i + di, nj = j + dj;
            if (!in_bounds_local(ni, nj)) continue;
            char ch = current_map[ni][nj];
            if (ch >= '0' && ch <= '8') {
              has_num_neighbor = true;
              int k = ch - '0';
              int unknown = 0, marked = 0;
              for (int di2 = -1; di2 <= 1; ++di2) {
                for (int dj2 = -1; dj2 <= 1; ++dj2) {
                  if (di2 == 0 && dj2 == 0) continue;
                  int nni = ni + di2, nnj = nj + dj2;
                  if (!in_bounds_local(nni, nnj)) continue;
                  if (current_map[nni][nnj] == '?') ++unknown;
                  else if (current_map[nni][nnj] == '@') ++marked;
                }
              }
              int rem = k - marked;
              if (rem <= 0) continue; // already handled by auto-explore earlier
              if (unknown > 0) score += static_cast<double>(rem) / unknown;
            }
          }
        }
        if (has_num_neighbor && score < best_score) {
          best_score = score;
          cand_r = i; cand_c = j;
        }
      }
    }
    if (cand_r != -1) goto FALLBACK_CHOSEN;
  }
  // D3: truly any '?'
  for (int i = 0; i < rows; ++i) {
    for (int j = 0; j < columns; ++j) {
      if (current_map[i][j] == '?') { cand_r = i; cand_c = j; goto FALLBACK_CHOSEN; }
    }
  }

FALLBACK_CHOSEN:
  if (cand_r != -1) {
    Execute(cand_r, cand_c, 0);
    return;
  }
}

#endif
