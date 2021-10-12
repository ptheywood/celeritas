//----------------------------------*-cu-*-----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file BetheHeitlerInteract.cu
//! \note Auto-generated by gen-interactor.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "base/Assert.hh"
#include "base/KernelParamCalculator.cuda.hh"
#include "../detail/BetheHeitlerLauncher.hh"

using namespace celeritas::detail;

namespace celeritas
{
namespace generated
{
namespace
{
__global__ void bethe_heitler_interact_kernel(
    const detail::BetheHeitlerDeviceRef ptrs,
    const ModelInteractRefs<MemSpace::device> model)
{
    auto tid = KernelParamCalculator::thread_id();
    if (!(tid < model.states.size()))
        return;

    detail::BetheHeitlerLauncher<MemSpace::device> launch(ptrs, model);
    launch(tid);
}
} // namespace

void bethe_heitler_interact(
    const detail::BetheHeitlerDeviceRef& ptrs,
    const ModelInteractRefs<MemSpace::device>& model)
{
    CELER_EXPECT(ptrs);
    CELER_EXPECT(model);

    static const KernelParamCalculator calc_kernel_params(
        bethe_heitler_interact_kernel, "bethe_heitler_interact");
    auto params = calc_kernel_params(model.states.size());
    bethe_heitler_interact_kernel<<<params.grid_size, params.block_size>>>(
        ptrs, model);
    CELER_CUDA_CHECK_ERROR();
}

} // namespace generated
} // namespace celeritas