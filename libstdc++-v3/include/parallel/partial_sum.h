// -*- C++ -*-

// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software
// Foundation; either version 2, or (at your option) any later
// version.

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this library; see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// As a special exception, you may use this file as part of a free
// software library without restriction.  Specifically, if other files
// instantiate templates or use macros or inline functions from this
// file, or you compile this file and link it with other files to
// produce an executable, this file does not by itself cause the
// resulting executable to be covered by the GNU General Public
// License.  This exception does not however invalidate any other
// reasons why the executable file might be covered by the GNU General
// Public License.

/** @file parallel/partial_sum.h
 *  @brief Parallel implementation of std::partial_sum(), i. e. prefix
 *  sums.
 *  This file is a GNU parallel extension to the Standard C++ Library.
 */

// Written by Johannes Singler.

#ifndef _GLIBCXX_PARALLEL_PARTIAL_SUM_H
#define _GLIBCXX_PARALLEL_PARTIAL_SUM_H 1

#include <omp.h>
#include <bits/stl_algobase.h>
#include <parallel/parallel.h>
#include <parallel/numericfwd.h>

namespace __gnu_parallel
{
  // Problem: there is no 0-element given.

  /** @brief Base case prefix sum routine.
   *  @param begin Begin iterator of input sequence.
   *  @param end End iterator of input sequence.
   *  @param result Begin iterator of output sequence.
   *  @param bin_op Associative binary function.
   *  @param value Start value. Must be passed since the neutral
   *  element is unknown in general.
   *  @return End iterator of output sequence. */
  template<typename InputIterator, typename OutputIterator, typename BinaryOperation>
  inline OutputIterator
  parallel_partial_sum_basecase(InputIterator begin, InputIterator end,
				OutputIterator result, BinaryOperation bin_op,
				typename std::iterator_traits<InputIterator>::value_type value)
  {
    if (begin == end)
      return result;

    while (begin != end)
      {
	value = bin_op(value, *begin);
	*result = value;
	result++;
	begin++;
      }
    return result;
  }

  /** @brief Parallel partial sum implmenetation, two-phase approach,
      no recursion.
      *  @param begin Begin iterator of input sequence.
      *  @param end End iterator of input sequence.
      *  @param result Begin iterator of output sequence.
      *  @param bin_op Associative binary function.
      *  @param n Length of sequence.
      *  @param num_threads Number of threads to use.
      *  @return End iterator of output sequence.
      */
  template<typename InputIterator, typename OutputIterator, typename BinaryOperation>
  OutputIterator
  parallel_partial_sum_linear(InputIterator begin, InputIterator end,
			      OutputIterator result, BinaryOperation bin_op,
			      typename std::iterator_traits<InputIterator>::difference_type n, int num_threads)
  {
    typedef std::iterator_traits<InputIterator> traits_type;
    typedef typename traits_type::value_type value_type;
    typedef typename traits_type::difference_type difference_type;

    if (num_threads > (n - 1))
      num_threads = static_cast<thread_index_t>(n - 1);
    if (num_threads < 2)
      {
	*result = *begin;
	return parallel_partial_sum_basecase(begin + 1, end, result + 1, bin_op, *begin);
      }

    difference_type* borders = static_cast<difference_type*>(__builtin_alloca(sizeof(difference_type) * (num_threads + 2)));

    if (Settings::partial_sum_dilatation == 1.0f)
      equally_split(n, num_threads + 1, borders);
    else
      {
	difference_type chunk_length = (int)((double)n / ((double)num_threads + Settings::partial_sum_dilatation)), borderstart = n - num_threads * chunk_length;
	borders[0] = 0;
	for (int i = 1; i < (num_threads + 1); i++)
	  {
	    borders[i] = borderstart;
	    borderstart += chunk_length;
	  }
	borders[num_threads + 1] = n;
      }

    value_type* sums = new value_type[num_threads];
    OutputIterator target_end;

#pragma omp parallel num_threads(num_threads)
    {
      int id = omp_get_thread_num();
      if (id == 0)
	{
	  *result = *begin;
	  parallel_partial_sum_basecase(begin + 1, begin + borders[1], result + 1, bin_op, *begin);
	  sums[0] = *(result + borders[1] - 1);
	}
      else
	{
	  sums[id] = std::accumulate(begin + borders[id] + 1, begin + borders[id + 1], *(begin + borders[id]), bin_op, __gnu_parallel::sequential_tag());
	}

#pragma omp barrier

#pragma omp single
      parallel_partial_sum_basecase(sums + 1, sums + num_threads, sums + 1, bin_op, sums[0]);

#pragma omp barrier

      // Still same team.
      parallel_partial_sum_basecase(begin + borders[id + 1], begin + borders[id + 2], result + borders[id + 1], bin_op, sums[id]);
    }

    delete[] sums;

    return result + n;
  }

  /** @brief Parallel partial sum front-end.
   *  @param begin Begin iterator of input sequence.
   *  @param end End iterator of input sequence.
   *  @param result Begin iterator of output sequence.
   *  @param bin_op Associative binary function.
   *  @return End iterator of output sequence. */
  template<typename InputIterator, typename OutputIterator, typename BinaryOperation>
  OutputIterator
  parallel_partial_sum(InputIterator begin, InputIterator end,
		       OutputIterator result, BinaryOperation bin_op)
  {
    _GLIBCXX_CALL(begin - end);

    typedef std::iterator_traits<InputIterator> traits_type;
    typedef typename traits_type::value_type value_type;
    typedef typename traits_type::difference_type difference_type;

    difference_type n = end - begin;

    int num_threads = get_max_threads();

    switch (Settings::partial_sum_algorithm)
      {
      case Settings::LINEAR:
	// Need an initial offset.
	return parallel_partial_sum_linear(begin, end, result, bin_op,
					   n, num_threads);
      default:
	// Partial_sum algorithm not implemented.
	_GLIBCXX_PARALLEL_ASSERT(0);
	return end;
      }
  }
}

#endif
