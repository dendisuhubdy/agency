#pragma once

#include <agency/cuda/grid_executor.hpp>
#include <agency/cuda/detail/bind.hpp>
#include <agency/detail/tuple.hpp>
#include <agency/functional.hpp>

namespace agency
{
namespace cuda
{
namespace detail
{


template<class Function>
struct block_executor_helper_functor
{
  Function f_;

  __device__
  void operator()(grid_executor::index_type idx)
  {
    agency::invoke(f_, agency::detail::get<1>(idx));
  }

  template<class T>
  __device__
  void operator()(grid_executor::index_type idx, agency::detail::unit, T& inner_shared_param)
  {
    agency::invoke(f_, agency::detail::get<1>(idx), inner_shared_param);
  }
};


}


class block_executor : private grid_executor
{
  private:
    using super_t = grid_executor;
    using super_traits = executor_traits<super_t>;
    using traits = executor_traits<block_executor>;

  public:
    using execution_category = concurrent_execution_tag;

    // XXX probably should be int
    using shape_type = unsigned int;
    using index_type = unsigned int;

    template<class T>
    using future = super_traits::future<T>;

    future<void> make_ready_future()
    {
      return super_traits::template make_ready_future<void>(*this);
    }

    using super_t::super_t;
    using super_t::device;

    template<class Function>
    __host__ __device__
    shape_type max_shape(Function f) const
    {
      return super_t::max_shape(f).y;
    }

    template<class Function, class Future, class Factory>
    future<void> then_execute(Function f, shape_type shape, Future& dependency, Factory factory)
    {
      auto g = detail::block_executor_helper_functor<Function>{f};
      return super_traits::then_execute(*this, g, super_t::shape_type{1,shape}, dependency, agency::detail::unit_factory(), factory);
    }

    template<class Function, class Factory>
    future<void> async_execute(Function f, shape_type shape, Factory factory)
    {
      auto ready = make_ready_future();
      return this->then_execute(f, shape, ready, factory);
    }

    template<class Function>
    future<void> async_execute(Function f, shape_type shape)
    {
      auto g = detail::block_executor_helper_functor<Function>{f};
      return super_traits::async_execute(*this, g, super_t::shape_type{1,shape});
    }

    template<class Function>
    void execute(Function f, shape_type shape)
    {
      this->async_execute(f, shape).wait();
    }

    template<class Function, class Factory>
    void execute(Function f, shape_type shape, Factory factory)
    {
      this->async_execute(f, shape, factory).wait();
    }
};


} // end cuda
} // end agency

