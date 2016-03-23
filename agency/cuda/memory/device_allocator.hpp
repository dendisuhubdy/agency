#pragma once

#include <agency/detail/config.hpp>
#include <agency/cuda/device.hpp>
#include <thrust/system_error.h>
#include <thrust/system/cuda/error.h>
#include <cuda_runtime.h>

namespace agency
{
namespace cuda
{


template<class T>
class device_allocator
{
  private:
    template<class> friend class device_allocator;

    device_id device_;

    const device_id& device() const
    {
      return device_;
    }

  public:
    using value_type = T;

    explicit device_allocator(const device_id& d)
      : device_(d)
    {}

    device_allocator()
      : device_allocator(all_devices()[0])
    {}

    device_allocator(const device_allocator&) = default;

    template<class U>
    device_allocator(const device_allocator<U>& other)
      : device_(other.device_)
    {}
  
    value_type* allocate(size_t n)
    {
      // switch to our device
      scoped_current_device set_current_device(device());

      value_type* result = nullptr;
  
      cudaError_t error = cudaMalloc(&result, n * sizeof(T));
  
      if(error != cudaSuccess)
      {
        throw thrust::system_error(error, thrust::cuda_category(), "device_allocator::allocate(): cudaMallocManaged");
      }
  
      return result;
    }
  
    void deallocate(value_type* ptr, size_t)
    {
      // switch to our device
      scoped_current_device set_current_device(device());

      cudaError_t error = cudaFree(ptr);
  
      if(error != cudaSuccess)
      {
        throw thrust::system_error(error, thrust::cuda_category(), "device_allocator::deallocate(): cudaFree");
      }
    }

    // XXX this should be implemented with a kernel launch or something
    template<class Iterator, class... Args>
    Iterator construct_each(Iterator first, Iterator last, Args&&... args)
    {
      //new(ptr) U(std::forward<Args>(args)...);
    }
};


} // end cuda
} // end agency
