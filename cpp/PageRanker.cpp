#include "PageRanker.h"
#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include <windows.h>
#include <iostream>

namespace fs = boost::filesystem;
namespace algo = boost::algorithm;

///////////////////////////////////////////////////////////////////////////////////////////////////
PageRanker::PageRanker() :
  epsilon_(1e-5),
  damping_(0.85) {}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::Calculate(const std::string & filename, size_t node_count, CallbackFunc func, bool mt) {
  func_ = func;
  node_count_ = node_count;
  if(!Prepare(filename)) {
    return;
  }

  if(mt) {
    MtPowerIteration();
  } else {
    PowerIteration();
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::Join() {
  
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PageRanker::Prepare(const std::string& filename) {
  if(!fs::exists(filename)) {
    func_(NodeRanks(), FileNotFound, 0);
    return false;
  }
  fs::ifstream if_stream(filename);
  std::string line;
  std::vector<std::string> strs;
  strs.resize(2);
  out_links_ = Links(node_count_, 0);
  links_to_ = LinksTo(node_count_);
  while(std::getline(if_stream, line)) {
    algo::split(strs, line, algo::is_any_of("\t"));
    int from = boost::lexical_cast<int>(strs[0]) - 1;
    ++(out_links_[from]);
    links_to_[boost::lexical_cast<int>(strs[1]) - 1].push_back(from);
  }
  return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::PowerIteration() {
  node_ranks_ = NodeRanks(node_count_, 1);
  for(size_t i = 0; i < node_count_; ++i) {
    node_ranks_[i] = links_to_[i].size();
  }
  bool contin = true;
  size_t iter_count = 0;

  DWORD start_tick = ::GetTickCount();
  while(contin) {
    contin = false;
    double delta, max_delta = -1e+20;
    for(size_t i = 0; i < node_count_; ++i) {
      double sum = 0;
      for each(int in_link in links_to_[i]) {
        sum += out_links_[in_link] ? node_ranks_[in_link] / out_links_[in_link] : 0;
      }
      delta = node_ranks_[i] - ((1 - damping_) + damping_ * sum);
      max_delta = (std::max)(delta, max_delta);
      node_ranks_[i] -= delta;
      contin |= delta > epsilon_;
    }
    ++iter_count;
    //std::cout << iter_count << " " << max_delta << std::endl;
  }
  std::cout << "Ticks:" << ::GetTickCount() - start_tick << std::endl;

  func_(node_ranks_, OK, iter_count);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::MtPowerIteration() {
  const Ranges& ranges = PrepareThreads();
  node_ranks_ = NodeRanks(node_count_, 1);
  tmp_ranks_ = NodeRanks(node_count_);
  converges_ = false;
  size_t iter_count = 0;

  DWORD start_tick = ::GetTickCount();
  while(!converges_) {
    converges_ = true;

    for(size_t i = 0; i < kThreadCount; ++i) {
      threads_[i].reset(new boost::thread(boost::bind(&PageRanker::ThreadFunc, this, ranges[i].first, ranges[i].second)));
    }

    for(size_t i = 0; i < kThreadCount; ++i) {
      threads_[i]->join();
    }

    node_ranks_.swap(tmp_ranks_);
    ++iter_count;
  }
  std::cout << "Ticks:" << ::GetTickCount() - start_tick << std::endl;
  func_(node_ranks_, OK, iter_count);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
PageRanker::Ranges PageRanker::PrepareThreads() {
  threads_.resize(kThreadCount);
  Ranges ranges(kThreadCount);
  int row_count = node_count_ / kThreadCount;
  for(size_t i = 0; i < kThreadCount; ++i) {
    ranges[i] = 
      std::make_pair(i * row_count,
                     i == kThreadCount - 1 ? node_count_ : ( (i + 1) * row_count));
  }
  return ranges;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::ThreadFunc(size_t first, size_t last) {
  for(size_t i = first; i < last; ++i) {
    double sum = 0;
    for each(int in_link in links_to_[i]) {
      sum += out_links_[in_link] ? node_ranks_[in_link] / out_links_[in_link] : 0;
    }
    tmp_ranks_[i] = (1 - damping_) + damping_ * sum;
    double delta = node_ranks_[i] - tmp_ranks_[i];
    converges_ &= delta < epsilon_;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////