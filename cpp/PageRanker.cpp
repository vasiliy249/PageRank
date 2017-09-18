#include "PageRanker.h"
#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/thread/scoped_thread.hpp"
#include <windows.h>
#include <iostream>

namespace fs = boost::filesystem;
namespace algo = boost::algorithm;

///////////////////////////////////////////////////////////////////////////////////////////////////
PageRanker::PageRanker() :
  epsilon_(1e-5),
  damping_(0.85),
  completed_threads_(0),
  iter_count_(0) {}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::StartCalc() {
  std::stringstream stm;
  stm << "*** Page ranker start with parameters:" << "\n" <<
    "\tFile with data: " << fs::canonical(fs::path(filename_)).string() << "\n" <<
    "\tNode count: " << node_count_ << "\n" <<
    "\tEpsilon: " << epsilon_ << "\n" <<
    "\tDamping factor:" << damping_ << "\n" <<
    "\tAlgorithm: ";
  if (calc_algo_ == CA_PowerIt) {
    stm << "Power iteration";
  } else if (calc_algo_ == CA_Gauss) {
    stm << "Gauss seidel";
  }
  stm << "\n" <<
    "\tThreads count: ";
  if (thread_count_ == 0 || thread_count_ == 1) {
    stm << "Main thread";
  } else {
    stm << thread_count_;
  }

  stm << "\n\n*** Start preparation of help structures...";
  std::cout << stm.str() << std::endl;
  if(!Prepare()) {
    return;
  }
  std::cout << "*** Preparation is complete.\n";

  DWORD start_tick = ::GetTickCount();
  if (calc_algo_ == CA_PowerIt) {
    if (thread_count_ == 0 || thread_count_ == 1) {
      PowerIteration();
    } else {
      MtPowerIteration1();
    }
  } else if (calc_algo_ == CA_Gauss) {
    GaussSeidel();
  }
  std::cout << "Ticks: " << ::GetTickCount() - start_tick << std::endl;

  func_(node_ranks_, OK, iter_count_);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::Join() {
  
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PageRanker::Prepare() {
  if(!fs::exists(filename_)) {
    func_(NodeRanks(), FileNotFound, 0);
    return false;
  }
  fs::ifstream if_stream(filename_);
  std::string line;
  std::vector<std::string> strs(2);
  out_links_ = Links(node_count_, 0);
  links_to_ = LinksTo(node_count_);
  while(std::getline(if_stream, line)) {
    algo::split(strs, line, algo::is_any_of("\t"));
    int from = boost::lexical_cast<int>(strs[0]) - 1;
    ++(out_links_[from]);
    links_to_[boost::lexical_cast<int>(strs[1]) - 1].push_back(from);
  }

  node_ranks_ = NodeRanks(node_count_, 1);
  tmp_ranks_ = NodeRanks(node_count_, 0);

  if (thread_count_ > 1) {
    PrepareThreads();
  }

  return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::PowerIteration() {
  do {
    for (size_t i = 0; i < node_count_; ++i) {
      double sum = 0;
      for each(int in_link in links_to_[i]) {
        sum += out_links_[in_link] ? node_ranks_[in_link] / out_links_[in_link] : 0;
      }
      tmp_ranks_[i] = (1 - damping_) + damping_ * sum;
    }
    ++iter_count_;
    node_ranks_.swap(tmp_ranks_);
  } while (!IsConverges());
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::GaussSeidel() {
  bool contin = true;
  while (contin) {
    contin = false;
    double new_rank;
    for (size_t i = 0; i < node_count_; ++i) {
      double sum = 0;
      for each(int in_link in links_to_[i]) {
        sum += out_links_[in_link] ? node_ranks_[in_link] / out_links_[in_link] : 0;
      }
      new_rank = (1 - damping_) + damping_ * sum;
      contin |= ::abs(node_ranks_[i] - new_rank) > epsilon_;
      node_ranks_[i] = new_rank;
    }
    ++iter_count_;
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::MtPowerIteration() {
  std::vector<int> ticks(thread_count_, 0);
  do {
    for (size_t i = 0; i < thread_count_; ++i) {
      threads_[i] = boost::thread(boost::bind(
        &PageRanker::ThreadFunc, this, thread_ranges_[i].first, thread_ranges_[i].second, &ticks[i]));
    }
    for (size_t i = 0; i < thread_count_; ++i) {
      threads_[i].join();
    }
    node_ranks_.swap(tmp_ranks_);
    ++iter_count_;
  } while (!IsConverges());

  for (size_t i = 0; i < thread_count_; ++i)
    std::cout << "ticks in thread: " << ticks[i] << std::endl;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::MtPowerIteration1() {
  converges_ = false;

  for (size_t i = 0; i < thread_count_; ++i) {
    threads_[i] = boost::thread(boost::bind(
      &PageRanker::ThreadFunc1, this, thread_ranges_[i].first, thread_ranges_[i].second));
  }

  do {
    {
      boost::mutex::scoped_lock lock(iter_mutex_);
      while (completed_threads_ < thread_count_) {
        Sleep(100);
      }
      ++iter_count_;
      if (IsConverges()) {
        converges_ = true;
        break;
      } else {
        converges_ = false;
        completed_threads_ = 0;
        node_ranks_.swap(tmp_ranks_);
      }
    }
    Sleep(100);
  } while (true);

  for (size_t i = 0; i < thread_count_; ++i) {
    threads_[i].join();
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::PrepareThreads() {
  threads_.resize(thread_count_);
  thread_ranges_.resize(thread_count_);
  int row_count = node_count_ / thread_count_;
  for(size_t i = 0; i < thread_count_; ++i) {
    thread_ranges_[i] = 
      std::make_pair(i * row_count,
                     i == thread_count_ - 1 ? node_count_ : ( (i + 1) * row_count));
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::ThreadFunc(size_t first, size_t last, int* ticks) {
  DWORD tick = ::GetTickCount();
  for(size_t i = first; i < last; ++i) {
    double sum = 0;
    for each(int in_link in links_to_[i]) {
      sum += out_links_[in_link] ? node_ranks_[in_link] / out_links_[in_link] : 0;
    }
    tmp_ranks_[i] = (1 - damping_) + damping_ * sum;
  }
  (*ticks) += ::GetTickCount() - tick;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
void PageRanker::ThreadFunc1(size_t first, size_t last) {
  do {
    for (size_t i = first; i < last; ++i) {
      double sum = 0;
      for each(int in_link in links_to_[i]) {
        sum += out_links_[in_link] ? node_ranks_[in_link] / out_links_[in_link] : 0;
      }
      tmp_ranks_[i] = (1 - damping_) + damping_ * sum;
    }
    completed_threads_++;
    boost::mutex::scoped_lock lock(iter_mutex_);
  } while (!converges_);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
bool PageRanker::IsConverges() {
  for (size_t i = 0; i < node_ranks_.size(); ++i) {
    if (::abs(node_ranks_[i] - tmp_ranks_[i]) > epsilon_) {
      return false;
    }
  }
  return true;
}