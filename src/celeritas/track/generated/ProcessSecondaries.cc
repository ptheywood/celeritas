//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/track/generated/ProcessSecondaries.cc
//! \note Auto-generated by gen-trackinit.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "celeritas/track/detail/ProcessSecondariesLauncher.hh" // IWYU pragma: associated
#include "corecel/sys/MultiExceptionHandler.hh"
#include "corecel/sys/ThreadId.hh"
#include "corecel/Types.hh"

namespace celeritas
{
namespace generated
{
void process_secondaries(
    const CoreHostRef& core_data)
{
    MultiExceptionHandler capture_exception;
    detail::ProcessSecondariesLauncher<MemSpace::host> launch(core_data);
    #pragma omp parallel for
    for (ThreadId::size_type i = 0; i < core_data.states.size(); ++i)
    {
        CELER_TRY_ELSE(launch(ThreadId{i}), capture_exception);
    }
    log_and_rethrow(std::move(capture_exception));
}

} // namespace generated
} // namespace celeritas
