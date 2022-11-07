//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/track/generated/LocateAlive.cc
//! \note Auto-generated by gen-trackinit.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "celeritas/track/detail/LocateAliveLauncher.hh" // IWYU pragma: associated
#include "corecel/sys/ThreadId.hh"
#include "corecel/Types.hh"

namespace celeritas
{
namespace generated
{
void locate_alive(
    const CoreHostRef& core_data,
    const TrackInitStateHostRef& init_data)
{
    detail::LocateAliveLauncher<MemSpace::host> launch(core_data, init_data);
    #pragma omp parallel for
    for (ThreadId::size_type i = 0; i < core_data.states.size(); ++i)
    {
        launch(ThreadId{i});
    }
}

} // namespace generated
} // namespace celeritas
