/**
 * Copyright 2022-2023, XGBoost contributors
 */
#pragma once

#include <gtest/gtest.h>

#include <chrono>  // for ms
#include <thread>  // for thread

#include "../../../../plugin/federated/federated_tracker.h"
#include "xgboost/json.h"  // for Json

namespace xgboost::collective {
template <typename WorkerFn>
void TestFederated(std::int32_t n_workers, WorkerFn&& fn) {
  Json config{Object()};
  config["federated_secure"] = Boolean{false};
  config["n_workers"] = Integer{n_workers};
  FederatedTracker tracker{config};
  auto fut = tracker.Run();

  std::vector<std::thread> workers;
  using namespace std::chrono_literals;
  while (tracker.Port() == 0) {
    std::this_thread::sleep_for(100ms);
  }
  std::int32_t port = tracker.Port();

  for (std::int32_t i = 0; i < n_workers; ++i) {
    workers.emplace_back([=] { fn(port, i); });
  }

  for (auto& t : workers) {
    t.join();
  }

  auto rc = tracker.Shutdown();
  ASSERT_TRUE(rc.OK()) << rc.Report();
  ASSERT_TRUE(fut.get().OK());
}
}  // namespace xgboost::collective