#include <array>
#include <chrono>
#include <iostream>
#include <mutex>
#include <stack>
#include <thread>

#include "board.h"
#include "brick.h"

using std::array;
using std::cout;
using std::endl;
using std::stack;

int PlaceBrick(const Brick& br, const Location& pos, int var_id, Board* bo) {
  return br.TryOccupy(pos, var_id, bo);
}

struct Record
{
  int var_id;
  Location pos_state;
};

static const int NUM_BRICKS = 12;
void SearchForSolution(const Location& start_point, const Location& end_point,
    std::vector<Board>* solutions, std::mutex* mu) {
  Board boards[NUM_BRICKS + 1];
  array<Brick, NUM_BRICKS> bricks;
  Brick::InitBricks(&bricks);
  cout << "Here are all the bricks:" << endl;
  for (int i = 0; i < bricks.size(); i++)
    bricks[i].Print();

  cout << "Here are the board:" << endl;
  boards[0].Print();

  Record rec;
  rec.var_id = -1; // Special case for the first brick.
  rec.pos_state = start_point;

  stack<Record> states;
  states.push(rec);

  int64_t loop_time = 0;
  int64_t max_loop = 5000000000L;
  auto start_time = std::chrono::steady_clock::now();
  auto time_stamp = start_time;
  while (++loop_time < max_loop && !states.empty()) {

    if (loop_time % 5000000 == 1) {
      cout << loop_time << endl;
      boards[states.size() - 1].Print();
    }
    if (loop_time % 5000000 == 1) {
      auto t = std::chrono::steady_clock::now();
      cout << "10000 round time consuming: " << std::chrono::duration_cast<std::chrono::milliseconds>(t - time_stamp).count() << " ms." << endl;
      time_stamp = t;
    }
    if (loop_time % 60000 == 1) {
      //std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    if (rec.pos_state.Valid()) { // Try to place next variant, except for the start point.
      Record new_rec(rec);
      //cout << "Try brick NO." << states.size() << " at location(" << rec.pos_state.x << ", " << rec.pos_state.y << ") with variant " << rec.var_id + 1 << ".";

      // Place the brick at the position.
      int var_id = PlaceBrick(bricks[states.size() - 1], rec.pos_state, rec.var_id + 1, &boards[states.size()]);
      if (var_id >= 0) { // One more done. Next brick.
        // cout << " Done with variant " << var_id << endl;
        states.top().var_id = var_id;

        if (states.size() >= NUM_BRICKS) { // Done.
          auto t = std::chrono::steady_clock::now();
          cout << "Solution " << solutions->size() + 1 << " found. Time used: " << std::chrono::duration_cast<std::chrono::seconds>(t - start_time).count() << " s." << endl;
          boards[NUM_BRICKS].Print();
          
          {
            std::unique_lock<std::mutex> lock(*mu);
            solutions->push_back(boards[NUM_BRICKS]);
          }

          // Pretend that we didn't successfully place bricks.
          boards[states.size()] = boards[states.size() - 1];
        } else {
          new_rec.pos_state = Location();  // Start searching from scratch.
          new_rec.var_id = -1;
          states.push(new_rec);
          rec = new_rec;
          boards[states.size()] = boards[states.size() - 1];

          continue;
        }
      }
      //cout << endl;
    }

    // Move to the next position.
    if (states.size() == 1)
      rec.pos_state = boards[0].GetNextVacancy(rec.pos_state, end_point);
    else
      rec.pos_state = boards[states.size() - 1].GetNextVacancy(rec.pos_state);

    if (rec.pos_state.Valid()) {
      states.top().pos_state = rec.pos_state;
      rec.var_id = -1;

      //cout << "Found a valid position(" << rec.pos_state.x << ", " << rec.pos_state.y << ") for brick NO." << states.size() << "." << endl;
    } else {
      if (states.size() == 1) {
        cout << "Searching finished." << endl;
        break;
      } else {
        //cout << "No proper position for brick NO." << states.size() + 1 << ". Fall back" << endl;
        boards[states.size() - 1] = boards[states.size() - 2];
        states.pop();
        rec = states.top();
      }
    }
  } // while
}

int main()
{
  auto start_time = std::chrono::steady_clock::now();
  Board::Init();

  std::mutex mu;
  std::vector<Board> solutions;

  const int num_threads = 8;
  std::vector<Location> check_points = Board::SplitRegion(num_threads);
  auto cp = check_points.begin();
  std::vector<std::thread> threads;
  
  for (int i = 0; i < num_threads; i++)
    threads.push_back(std::thread(SearchForSolution, *cp, *(++cp), &solutions, &mu));
  
  for (auto& t : threads)
    t.join();

  cout << "Here are all the " << solutions.size() << "solutions:" << endl;
  for (auto& s : solutions)
    s.Print();

  auto t = std::chrono::steady_clock::now();
  cout << solutions.size() << " solutions found. Time used: " << std::chrono::duration_cast<std::chrono::seconds>(t - start_time).count() << " s." << endl;
}




