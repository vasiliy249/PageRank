#ifndef SRC_PAGE_RANKER_H
#define SRC_PAGE_RANKER_H

#include <string>
#include <vector>
#include <atomic>
#include "boost/function.hpp"
#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"

const size_t kThreadCount = 2;

enum CalcAlgo {
  CA_PowerIt = 0,
  CA_Gauss
};

class PageRanker {
public:
  enum ErrorCode { Undef = 0, FileNotFound, IncorrectFile, OK };
  typedef std::vector<double> NodeRanks;
  typedef boost::function<void(const PageRanker::NodeRanks& ranks, PageRanker::ErrorCode code, size_t iter_count)> CallbackFunc;
  typedef std::vector<int> Links;
  typedef std::vector<std::vector<int>> LinksTo;
  typedef std::vector<boost::thread> ThreadVec;
  typedef std::vector< std::pair<size_t, size_t> > Ranges;

  PageRanker();
  void SetEpsilon(double epsilon) { epsilon_ = epsilon; }
  void SetDamping(double damping) { damping_ = damping; }
  void SetFile(const std::string& filename) { filename_ = filename; }
  void SetNodeCount(int node_count) { node_count_ = node_count; }
  void SetAlgo(CalcAlgo calc_algo) { calc_algo_ = calc_algo; }
  void SetThreadCount(int count) { thread_count_ = count; }
  void SetCallback(CallbackFunc func) { func_ = func; }

  void StartCalc();

  void Join();

private:
  bool Prepare();
  void PowerIteration();
  void GaussSeidel();
  void MtPowerIteration();
  void MtPowerIteration1();
  void PrepareThreads();
  void ThreadFunc(size_t first, size_t last, int* ticks);
  void ThreadFunc1(size_t first, size_t last);
  bool IsConverges();

private:
  /*arguments*/
  double            epsilon_;
  double            damping_;
  std::string       filename_;
  size_t            node_count_;
  CalcAlgo          calc_algo_;
  int               thread_count_;
  CallbackFunc      func_;

  /*help structures*/
  Links             out_links_;
  LinksTo           links_to_;

  NodeRanks         tmp_ranks_;
  NodeRanks         node_ranks_;
  int               iter_count_;

  ThreadVec         threads_;
  Ranges            thread_ranges_;

  std::atomic<bool> converges_;
  std::atomic<int>  completed_threads_;
  boost::mutex      iter_mutex_;
};

#endif // !SRC_PAGE_RANKER_H
