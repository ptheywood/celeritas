//----------------------------------*-C++-*----------------------------------//
// Copyright 2023 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/global/CoreState.hh
//---------------------------------------------------------------------------//
#pragma once

#include "corecel/Types.hh"
#include "corecel/data/CollectionStateStore.hh"
#include "corecel/data/DeviceVector.hh"
#include "corecel/sys/ThreadId.hh"
#include "celeritas/global/CoreTrackData.hh"

namespace celeritas
{
class CoreParams;
//---------------------------------------------------------------------------//
/*!
 * Store all state data for a single thread.
 */
template<MemSpace M>
class CoreState
{
  public:
    //!@{
    //! \name Type aliases
    using Ref = CoreStateData<Ownership::reference, M>;
    using Ptr = ObserverPtr<Ref, M>;
    using size_type = TrackSlotId::size_type;
    //!@}

  public:
    // Construct from CoreParams
    CoreState(CoreParams const& params,
              StreamId stream_id,
              size_type num_track_slots);

    //! Thread/stream ID
    StreamId stream_id() const { return this->ref().stream_id; }

    //! Number of track slots
    size_type size() const { return states_.size(); }

    //! Get a reference to the mutable state data
    Ref& ref() { return states_.ref(); }

    //! Get a reference to the mutable state data
    Ref const& ref() const { return states_.ref(); }

    // Get a native-memspace pointer to the mutable state data
    inline Ptr ptr();

  private:
    // State data
    CollectionStateStore<CoreStateData, M> states_;

    // Copy of state ref in device memory
    DeviceVector<Ref> device_ref_vec_;
};

//---------------------------------------------------------------------------//
/*!
 * Access a native pointer to a NativeCRef.
 *
 * This way, CUDA kernels only need to copy a pointer in the kernel arguments,
 * rather than the entire (rather large) DeviceRef object.
 */
template<MemSpace M>
auto CoreState<M>::ptr() -> Ptr
{
    if constexpr (M == MemSpace::host)
    {
        return make_observer(&this->ref());
    }
    else if constexpr (M == MemSpace::device)
    {
        CELER_ENSURE(!device_ref_vec_.empty());
        return make_observer(device_ref_vec_);
    }
}

//---------------------------------------------------------------------------//
}  // namespace celeritas