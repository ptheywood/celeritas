//----------------------------------*-cu-*-----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file SeltzerBergerInteract.cu
//! \note Auto-generated by gen-interactor.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "base/Assert.hh"
#include "base/KernelParamCalculator.cuda.hh"
#include "../detail/SeltzerBergerLauncher.hh"

using namespace celeritas::detail;

namespace celeritas
{
namespace generated
{
namespace
{
__global__ void seltzer_berger_interact_kernel(
    const detail::SeltzerBergerDeviceRef ptrs,
    const ModelInteractRefs<MemSpace::device> model)
{
    auto tid = KernelParamCalculator::thread_id();
    if (!(tid < model.states.size()))
        return;

    detail::SeltzerBergerLauncher<MemSpace::device> launch(ptrs, model);
    launch(tid);
}
} // namespace

void seltzer_berger_interact(
    const detail::SeltzerBergerDeviceRef& ptrs,
    const ModelInteractRefs<MemSpace::device>& model)
{
    CELER_EXPECT(ptrs);
    CELER_EXPECT(model);

    static const KernelParamCalculator calc_kernel_params(
        seltzer_berger_interact_kernel, "seltzer_berger_interact");
    auto params = calc_kernel_params(model.states.size());
    seltzer_berger_interact_kernel<<<params.grid_size, params.block_size>>>(
        ptrs, model);
    CELER_CUDA_CHECK_ERROR();
}

} // namespace generated
} // namespace celeritas