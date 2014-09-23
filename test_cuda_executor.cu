#include "cuda/cuda_executor.hpp"

struct hello_world
{
  __device__
  void operator()(uint2 index)
  {
    printf("Hello world from block %d, thread %d\n", index.x, index.y);
  }
};


struct with_shared_arg
{
  __device__
  void operator()(uint2 index, thrust::tuple<int&,int&> shared_arg)
  {
    int& outer_shared = thrust::get<0>(shared_arg);
    int& inner_shared = thrust::get<1>(shared_arg);

    atomicAdd(&outer_shared, 1);
    atomicAdd(&inner_shared, 1);

    __syncthreads();

    if(index.y == 0)
    {
      printf("outer_shared: %d\n", outer_shared);
      printf("inner_shared: %d\n", inner_shared);
    }
  }
};


__host__ __device__
void launch_nested_kernel()
{
  cuda_executor ex;
  bulk_invoke(ex, make_uint2(2,2), hello_world());
}


__global__ void kernel()
{
  launch_nested_kernel();
}


template<typename T>
__host__ __device__
void maybe_launch_nested_kernel()
{
#if __cuda_lib_has_cudart
  launch_nested_kernel();
#else
  printf("sorry, can't launch a kernel\n");
#endif
}


template<typename T>
__global__ void kernel_template()
{
  printf("kernel_template\n");
  maybe_launch_nested_kernel<T>();
}

int main()
{
  cuda_executor ex;

  std::cout << "Testing bulk_invoke on host" << std::endl;
  bulk_invoke(ex, make_uint2(2,2), hello_world());
  cudaDeviceSynchronize();

  std::cout << "Testing bulk_invoke with shared arg on host" << std::endl;
  ex.bulk_invoke(with_shared_arg(), make_uint2(2,2), thrust::make_tuple(7,13));
  cudaDeviceSynchronize();

  std::cout << "Testing bulk_invoke() on device" << std::endl;
  kernel<<<1,1>>>();
  cudaDeviceSynchronize();

  std::cout << "Testing bulk_invoke() on device from kernel template" << std::endl;
  kernel_template<int><<<1,1>>>();
  cudaDeviceSynchronize();

  return 0;
}

