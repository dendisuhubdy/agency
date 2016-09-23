#include <agency/agency.hpp>
#include <agency/execution/executor/detail/new_executor_traits.hpp>
#include <iostream>

#include "../test_executors.hpp"


template<class Executor>
void test_with_void_predecessor_returning_void(Executor exec)
{
  using namespace agency::detail::new_executor_traits_detail;

  agency::new_executor_shape_t<Executor> shape{100};

  agency::new_executor_future_t<Executor,void> predecessor = agency::future_traits<agency::new_executor_future_t<Executor,void>>::make_ready();
  
  int shared_arg = 0;
  
  int increment_me = 0;
  std::mutex mut;
  auto fut = bulk_then_execute_with_auto_result(exec, [&](size_t idx, int& shared_arg)
  {
    mut.lock();
    increment_me += 1;
    ++shared_arg;
    mut.unlock();
  },
  shape,
  predecessor,
  [&]
  {
    return std::ref(shared_arg);
  });
  
  fut.wait();
  
  assert(increment_me == shape);
  assert(shared_arg == shape);
}


template<class Executor>
void test_with_void_predecessor_returning_results(Executor exec)
{
  using namespace agency::detail::new_executor_traits_detail;

  auto predecessor_future = agency::detail::make_ready_future();

  using shape_type = agency::new_executor_shape_t<Executor>;
  using index_type = agency::new_executor_index_t<Executor>;

  size_t shape = 10;
  
  auto f = bulk_then_execute_with_auto_result(exec,
    [](index_type idx, std::vector<int>& shared_arg)
    {
      return shared_arg[idx];
    },
    shape,
    predecessor_future,
    [=]{ return std::vector<int>(shape, 13); }  // shared_arg
  );
  
  auto result = f.get();
  
  using container_type = agency::new_executor_container_t<Executor,int>;
  assert(container_type(shape, 13) == result);
}


template<class Executor>
void test_with_non_void_predecessor_returning_void(Executor exec)
{
  using namespace agency::detail::new_executor_traits_detail;

  agency::new_executor_shape_t<Executor> shape{100};

  agency::new_executor_future_t<Executor,int> predecessor_future = agency::future_traits<agency::new_executor_future_t<Executor,int>>::template make_ready<int>(13);
  
  int shared_arg = 0;
  
  int increment_me = 0;
  std::mutex mut;
  auto fut = bulk_then_execute_with_auto_result(exec, [&](size_t idx, int& predecessor, int& shared_arg)
  {
    mut.lock();
    increment_me += predecessor;
    ++shared_arg;
    mut.unlock();
  },
  shape,
  predecessor_future,
  [&]
  {
    return std::ref(shared_arg);
  });
  
  fut.wait();
  
  assert(increment_me == shape * 13);
  assert(shared_arg == shape);
}


template<class Executor>
void test_with_non_void_predecessor_returning_results(Executor exec)
{
  using namespace agency::detail::new_executor_traits_detail;

  agency::new_executor_future_t<Executor,int> predecessor_future = agency::future_traits<agency::new_executor_future_t<Executor,int>>::template make_ready<int>(7);

  using shape_type = agency::new_executor_shape_t<Executor>;
  using index_type = agency::new_executor_index_t<Executor>;

  size_t shape = 10;
  
  auto f = bulk_then_execute_with_auto_result(exec,
    [](index_type idx, int& predecessor, std::vector<int>& shared_arg)
    {
      return predecessor + shared_arg[idx];
    },
    shape,
    predecessor_future,
    [=]{ return std::vector<int>(shape, 13); }  // shared_arg
  );
  
  auto result = f.get();
  
  using container_type = agency::new_executor_container_t<Executor,int>;
  assert(container_type(shape, 7 + 13) == result);
}


int main()
{
  test_with_void_predecessor_returning_void(bulk_synchronous_executor());
  test_with_void_predecessor_returning_void(bulk_asynchronous_executor());
  test_with_void_predecessor_returning_void(bulk_continuation_executor());
  test_with_void_predecessor_returning_void(not_a_bulk_synchronous_executor());
  test_with_void_predecessor_returning_void(not_a_bulk_asynchronous_executor());
  test_with_void_predecessor_returning_void(not_a_bulk_continuation_executor());
  test_with_void_predecessor_returning_void(complete_bulk_executor());

  test_with_void_predecessor_returning_results(bulk_synchronous_executor());
  test_with_void_predecessor_returning_results(bulk_asynchronous_executor());
  test_with_void_predecessor_returning_results(bulk_continuation_executor());
  test_with_void_predecessor_returning_results(not_a_bulk_synchronous_executor());
  test_with_void_predecessor_returning_results(not_a_bulk_asynchronous_executor());
  test_with_void_predecessor_returning_results(not_a_bulk_continuation_executor());
  test_with_void_predecessor_returning_results(complete_bulk_executor());

  test_with_non_void_predecessor_returning_void(bulk_synchronous_executor());
  test_with_non_void_predecessor_returning_void(bulk_asynchronous_executor());
  test_with_non_void_predecessor_returning_void(bulk_continuation_executor());
  test_with_non_void_predecessor_returning_void(not_a_bulk_synchronous_executor());
  test_with_non_void_predecessor_returning_void(not_a_bulk_asynchronous_executor());
  test_with_non_void_predecessor_returning_void(not_a_bulk_continuation_executor());
  test_with_non_void_predecessor_returning_void(complete_bulk_executor());

  test_with_non_void_predecessor_returning_results(bulk_synchronous_executor());
  test_with_non_void_predecessor_returning_results(bulk_asynchronous_executor());
  test_with_non_void_predecessor_returning_results(bulk_continuation_executor());
  test_with_non_void_predecessor_returning_results(not_a_bulk_synchronous_executor());
  test_with_non_void_predecessor_returning_results(not_a_bulk_asynchronous_executor());
  test_with_non_void_predecessor_returning_results(not_a_bulk_continuation_executor());
  test_with_non_void_predecessor_returning_results(complete_bulk_executor());

  std::cout << "OK" << std::endl;
  
  return 0;
}

