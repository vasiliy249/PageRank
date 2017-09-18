#include "PageRanker.h"
#include "boost/lexical_cast.hpp"
#include "boost/program_options.hpp"
#include <iostream>

namespace po = boost::program_options;

void Callback(const PageRanker::NodeRanks& ranks, PageRanker::ErrorCode code, size_t iter_count) {
  if(code != PageRanker::OK) {
    std::cout << "error";
    return;
  } else {
    std::cout << "Iter count: " << iter_count << std::endl;
  }
}

int main(int argc, char* argv[]) {
  std::string path_to_data;
  int rel_count;
  double epsilon;
  double damping;
  CalcAlgo calc_algo;
  int thread_count;

  po::options_description desc("General options");
  desc.add_options()
    ("help,h", "Show help")
    ("file,f", po::value<std::string>(&path_to_data), "Input file with graph relations")
    ("count,c", po::value<int>(&rel_count), "Count of nodes")
    ("eps,e", po::value<double>(&epsilon), "Epsilon")
    ("damping,d", po::value<double>(&damping), "Damping factor")
    ("algo,a", po::value<int>((int*)(&calc_algo)), "Algorithm (0 = power iteration, 1 = gauss seidel)")
    ("threads,t", po::value<int>(&thread_count), "How much threads is used to perform algorithm");
  po::variables_map vm;

  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    if (vm.count("help")) {
      std::cout << desc << std::endl;
      return 1;
    }
  } catch (...) {
    std::cout << "error, usage: -h" << std::endl;
    return 0;
  }

  PageRanker page_ranker;
  page_ranker.SetFile(path_to_data);
  page_ranker.SetEpsilon(epsilon);
  page_ranker.SetDamping(damping);
  page_ranker.SetNodeCount(rel_count);
  page_ranker.SetAlgo(calc_algo);
  page_ranker.SetThreadCount(thread_count);
  page_ranker.SetCallback(&Callback);
  page_ranker.StartCalc();
  //page-rank.txt", 11
  //web-Stanford.txt", 281903
  page_ranker.Join();

  return 0;
}

