//----------------------------------*-cc-*-----------------------------------//
// Copyright 2021 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file MuBremsstrahlungInteract.cc
//! \note Auto-generated by gen-interactor.py: DO NOT MODIFY!
//---------------------------------------------------------------------------//
#include "base/Assert.hh"
#include "base/Range.hh"
#include "base/Types.hh"
#include "../detail/MuBremsstrahlungLauncher.hh"

namespace celeritas
{
namespace generated
{
void mu_bremsstrahlung_interact(
    const detail::MuBremsstrahlungHostRef& ptrs,
    const ModelInteractRefs<MemSpace::host>& model)
{
    CELER_EXPECT(ptrs);
    CELER_EXPECT(model);

    detail::MuBremsstrahlungLauncher<MemSpace::host> launch(ptrs, model);
    for (auto tid : range(ThreadId{model.states.size()}))
    {
        launch(tid);
    }
}

} // namespace generated
} // namespace celeritas