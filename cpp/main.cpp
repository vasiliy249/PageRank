#include "PageRanker.h"
#include <iostream>

void Callback(const PageRanker::NodeRanks& ranks, PageRanker::ErrorCode code, size_t iter_count) {
  if(code != PageRanker::OK) {
    std::cout << "error";
    return;
  } else {
    std::cout << "Iter count: " << iter_count << std::endl;
  }
}

int main() {
  PageRanker page_ranker;
  page_ranker.SetEpsilon(1e-10);
  page_ranker.SetDamping(0.85);

  
  //page_ranker.Calculate("C:\\Prj\\Py\\PageRankTest\\page-rank.txt", 11, &Callback, false);
  page_ranker.Calculate("C:\\Prj\\Py\\PageRankTest\\web-Stanford.txt", 281903, &Callback, true);
  page_ranker.Join();

  system("pause");
  return 0;
}

