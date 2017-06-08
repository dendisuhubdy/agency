#pragma once

#include <agency/detail/config.hpp>
#include <agency/detail/requires.hpp>
#include <agency/detail/algorithm/uninitialized_copy.hpp>

namespace agency
{
namespace detail
{
namespace overlapped_uninitialized_copy_detail
{


template<class Allocator, class Iterator1, class Iterator2>
__AGENCY_ANNOTATION
Iterator2 uninitialized_copy_backward(Allocator& alloc, Iterator1 first, Iterator1 last, Iterator2 result)
{
  // yes, we preincrement
  // the ranges are open on the right, i.e. [first, last)
  while(first != last)
  {
    agency::detail::allocator_traits<Allocator>::construct(alloc, &*--result, *--last);
  }

  return result;
}


} // end overlapped_uninitialized_copy_detail


template<class ExecutionPolicy, class Allocator, class Iterator,
         __AGENCY_REQUIRES(
           !policy_is_sequenced<decay_t<ExecutionPolicy>>::value and
           std::is_convertible<
             typename std::iterator_traits<Iterator>::iterator_category,
             std::random_access_iterator_tag
           >::value
         )>
__AGENCY_ANNOTATION
Iterator overlapped_uninitialized_copy(ExecutionPolicy&& policy, Allocator& alloc, Iterator first, Iterator last, Iterator result)
{
  if(first < last && first <= result && result < last)
  {
    // result lies in [first, last)
    // it's safe to use uninitialized_copy_backward here
    overlapped_uninitialized_copy_detail::uninitialized_copy_backward(alloc, first, last, result + (last - first));
    result += (last - first);
  }
  else
  {
    // result + (last - first) lies in [first, last)
    // it's safe to use uninitialized_copy here
    result = agency::detail::uninitialized_copy(std::forward<ExecutionPolicy>(policy), alloc, first, last, result);
  } // end else

  return result;
}


template<class ExecutionPolicy, class Allocator, class Iterator,
         __AGENCY_REQUIRES(
           policy_is_sequenced<decay_t<ExecutionPolicy>>::value or
           !std::is_convertible<
             typename std::iterator_traits<Iterator>::iterator_category,
             std::random_access_iterator_tag
           >::value
         )>
__AGENCY_ANNOTATION
Iterator overlapped_uninitialized_copy(ExecutionPolicy&&, Allocator& alloc, Iterator first, Iterator last, Iterator result)
{
  if(first < last && first <= result && result < last)
  {
    // result lies in [first, last)
    // it's safe to use uninitialized_copy_backward here
    overlapped_uninitialized_copy_detail::uninitialized_copy_backward(alloc, first, last, result + (last - first));
    result += (last - first);
  }
  else
  {
    // result + (last - first) lies in [first, last)
    // it's safe to use uninitialized_copy here
    agency::sequenced_execution_policy seq;
    result = agency::detail::uninitialized_copy(seq, alloc, first, last, result);
  } // end else

  return result;
}


template<class Allocator, class Iterator>
__AGENCY_ANNOTATION
Iterator overlapped_uninitialized_copy(Allocator& alloc, Iterator first, Iterator last, Iterator result)
{
  // pass this instead of agency::seq to work around the prohibition on
  // taking the address of a global constexpr object (i.e., agency::seq) from a CUDA __device__ function
  agency::sequenced_execution_policy seq;
  return detail::overlapped_uninitialized_copy(seq, alloc, first, last, result);
}


} // end detail
} // end agency

