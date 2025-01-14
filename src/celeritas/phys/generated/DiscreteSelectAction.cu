//---------------------------------*-CUDA-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/phys/generated/DiscreteSelectAction.cu
//! \note Auto-generated by gen-action.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "DiscreteSelectAction.hh"

#include "corecel/device_runtime_api.h"
#include "corecel/Assert.hh"
#include "corecel/Types.hh"
#include "corecel/sys/KernelParamCalculator.device.hh"
#include "corecel/sys/Device.hh"
#include "celeritas/global/TrackLauncher.hh"
#include "../detail/DiscreteSelectActionImpl.hh"

namespace celeritas
{
namespace generated
{
namespace
{
__global__ void discrete_select_kernel(CoreDeviceRef const data
)
{
    auto tid = KernelParamCalculator::thread_id();
    if (!(tid < data.states.size()))
        return;

    auto launch = make_track_launcher(data, detail::discrete_select_track);
    launch(tid);
}
} // namespace

void DiscreteSelectAction::execute(const CoreDeviceRef& data) const
{
    CELER_EXPECT(data);
    CELER_LAUNCH_KERNEL(discrete_select,
                        celeritas::device().default_block_size(),
                        data.states.size(),
                        data);
}

} // namespace generated
} // namespace celeritas
