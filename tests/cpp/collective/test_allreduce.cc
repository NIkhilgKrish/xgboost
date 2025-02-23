/**
 * Copyright 2023, XGBoost Contributors
 */
#include <gtest/gtest.h>

#include "../../../src/collective/allreduce.h"
#include "../../../src/collective/tracker.h"
#include "test_worker.h"  // for WorkerForTest, TestDistributed

namespace xgboost::collective {

namespace {
class AllreduceWorker : public WorkerForTest {
 public:
  using WorkerForTest::WorkerForTest;

  void Basic() {
    {
      std::vector<double> data(13, 0.0);
      Allreduce(comm_, common::Span{data.data(), data.size()}, [](auto lhs, auto rhs) {
        for (std::size_t i = 0; i < rhs.size(); ++i) {
          rhs[i] += lhs[i];
        }
      });
      ASSERT_EQ(std::accumulate(data.cbegin(), data.cend(), 0.0), 0.0);
    }
    {
      std::vector<double> data(1, 1.0);
      Allreduce(comm_, common::Span{data.data(), data.size()}, [](auto lhs, auto rhs) {
        for (std::size_t i = 0; i < rhs.size(); ++i) {
          rhs[i] += lhs[i];
        }
      });
      ASSERT_EQ(data[0], static_cast<double>(comm_.World()));
    }
  }

  void Acc() {
    std::vector<double> data(314, 1.5);
    Allreduce(comm_, common::Span{data.data(), data.size()}, [](auto lhs, auto rhs) {
      for (std::size_t i = 0; i < rhs.size(); ++i) {
        rhs[i] += lhs[i];
      }
    });
    for (std::size_t i = 0; i < data.size(); ++i) {
      auto v = data[i];
      ASSERT_EQ(v, 1.5 * static_cast<double>(comm_.World())) << i;
    }
  }
};

class AllreduceTest : public SocketTest {};
}  // namespace

TEST_F(AllreduceTest, Basic) {
  std::int32_t n_workers = std::min(7u, std::thread::hardware_concurrency());
  TestDistributed(n_workers, [=](std::string host, std::int32_t port, std::chrono::seconds timeout,
                                 std::int32_t r) {
    AllreduceWorker worker{host, port, timeout, n_workers, r};
    worker.Basic();
  });
}

TEST_F(AllreduceTest, Sum) {
  std::int32_t n_workers = std::min(7u, std::thread::hardware_concurrency());
  TestDistributed(n_workers, [=](std::string host, std::int32_t port, std::chrono::seconds timeout,
                                 std::int32_t r) {
    AllreduceWorker worker{host, port, timeout, n_workers, r};
    worker.Acc();
  });
}
}  // namespace xgboost::collective
