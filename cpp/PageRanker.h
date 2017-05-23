#ifndef SRC_PAGE_RANKER_H
#define SRC_PAGE_RANKER_H

#include <string>
#include <vector>
#include "boost/function.hpp"

class PageRanker {
public:
  enum ErrorCode { Undef = 0, FileNotFound, IncorrectFile, OK };
  typedef std::vector<double> NodeRanks;
  typedef boost::function<void(const PageRanker::NodeRanks& ranks, PageRanker::ErrorCode code, size_t iter_count)> CallbackFunc;
  typedef std::vector<int> Links;
  typedef std::vector<std::vector<int>> LinksTo;

  PageRanker();
  void SetEpsilon(double epsilon) { epsilon_ = epsilon; }
  void SetDamping(double damping) { damping_ = damping; }

  void Calculate(const std::string& filename, size_t node_count, CallbackFunc func, bool mt);
  void Join();

private:
  bool Prepare(const std::string & filename);
  void PowerIteration();

private:
  CallbackFunc func_;
  double       epsilon_;
  double       damping_;
  Links        out_links_;
  LinksTo      links_to_;
  NodeRanks    node_ranks_;
  size_t       node_count_;
};

#endif // !SRC_PAGE_RANKER_H
