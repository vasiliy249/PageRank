#ifndef SRC_PAGE_RANKER_H
#define SRC_PAGE_RANKER_H

#include <string>
#include <vector>
#include "boost/function.hpp"
#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"

const size_t kThreadCount = 8;

class PageRanker {
public:
  enum ErrorCode { Undef = 0, FileNotFound, IncorrectFile, OK };
  typedef std::vector<double> NodeRanks;
  typedef boost::function<void(const PageRanker::NodeRanks& ranks, PageRanker::ErrorCode code, size_t iter_count)> CallbackFunc;
  typedef std::vector<int> Links;
  typedef std::vector<std::vector<int>> LinksTo;
  typedef std::vector<boost::shared_ptr<boost::thread>> ThreadVec;
  typedef std::vector<std::pair<size_t, size_t>> Ranges;

  PageRanker();
  void SetEpsilon(double epsilon) { epsilon_ = epsilon; }
  void SetDamping(double damping) { damping_ = damping; }

  void Calculate(const std::string& filename, size_t node_count, CallbackFunc func, bool mt);
  void Join();

private:
  bool Prepare(const std::string & filename);
  void PowerIteration();
  void MtPowerIteration();
  Ranges PrepareThreads();
  void ThreadFunc(size_t first, size_t last);

private:
  CallbackFunc func_;
  double       epsilon_;
  double       damping_;
  Links        out_links_;
  LinksTo      links_to_;
  NodeRanks    node_ranks_;
  size_t       node_count_;
  NodeRanks    tmp_ranks_;
  ThreadVec    threads_;
  bool         converges_;
};

#endif // !SRC_PAGE_RANKER_H
