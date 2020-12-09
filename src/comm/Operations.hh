//----------------------------------*-C++-*----------------------------------//
// Copyright 2020 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file Operations.hh
//---------------------------------------------------------------------------//
#pragma once

#include <type_traits>
#include "base/Span.hh"
#include "Communicator.hh"

namespace celeritas
{
//---------------------------------------------------------------------------//
// TYPES
//---------------------------------------------------------------------------//
enum class Operation
{
    min,
    max,
    sum,
    prod,
};

//---------------------------------------------------------------------------//
// FREE FUNCTIONS
//---------------------------------------------------------------------------//
// Wait for all processes in this communicator to reach the barrier
inline void barrier(const Communicator& comm);

//---------------------------------------------------------------------------//
// All-to-all reduction on the data from src to dst
template<class T, std::size_t N>
inline void allreduce(const Communicator& comm,
                      Operation           op,
                      span<const T, N>    src,
                      span<T, N>          dst);

//---------------------------------------------------------------------------//
// All-to-all reduction on the data, in place
template<class T, std::size_t N>
inline void allreduce(const Communicator& comm, Operation op, span<T, N> data);

//---------------------------------------------------------------------------//
// Perform reduction on a fundamental scalar and return the result
template<class T, std::enable_if_t<std::is_fundamental<T>::value, T*> = nullptr>
inline T allreduce(const Communicator& comm, Operation op, const T src);

//---------------------------------------------------------------------------//
} // namespace celeritas

#include "Operations.i.hh"
