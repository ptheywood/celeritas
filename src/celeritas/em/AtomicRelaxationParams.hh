//----------------------------------*-C++-*----------------------------------//
// Copyright 2020-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/em/AtomicRelaxationParams.hh
//---------------------------------------------------------------------------//
#pragma once

#include <functional>

#include "corecel/Types.hh"
#include "corecel/data/CollectionMirror.hh"
#include "celeritas/Quantities.hh"
#include "celeritas/io/ImportAtomicRelaxation.hh"
#include "celeritas/phys/AtomicNumber.hh"

#include "data/AtomicRelaxationData.hh"

namespace celeritas
{
class CutoffParams;
class MaterialParams;
class ParticleParams;

//---------------------------------------------------------------------------//
/*!
 * Data management for the EADL transition data for atomic relaxation.
 */
class AtomicRelaxationParams
{
  public:
    //@{
    //! Type aliases
    using HostRef        = HostCRef<AtomicRelaxParamsData>;
    using DeviceRef      = DeviceCRef<AtomicRelaxParamsData>;
    using MevEnergy      = units::MevEnergy;
    using ReadData       = std::function<ImportAtomicRelaxation(AtomicNumber)>;
    using SPConstCutoffs = std::shared_ptr<const CutoffParams>;
    using SPConstMaterials = std::shared_ptr<const MaterialParams>;
    using SPConstParticles = std::shared_ptr<const ParticleParams>;
    //@}

    struct Input
    {
        SPConstCutoffs   cutoffs;
        SPConstMaterials materials;
        SPConstParticles particles;
        ReadData         load_data;
        bool is_auger_enabled{false}; //!< Whether to produce Auger electrons
    };

  public:
    // Construct with a vector of element identifiers
    explicit AtomicRelaxationParams(const Input& inp);

    // Access EADL data on the host
    const HostRef& host_ref() const { return data_.host(); }

    // Access EADL data on the device
    const DeviceRef& device_ref() const { return data_.device(); }

  private:
    // Whether to simulate non-radiative transitions
    bool is_auger_enabled_;

    // Host/device storage and reference
    CollectionMirror<AtomicRelaxParamsData> data_;

    // HELPER FUNCTIONS
    using HostData = HostVal<AtomicRelaxParamsData>;
    void append_element(const ImportAtomicRelaxation& inp,
                        HostData*                     data,
                        MevEnergy                     electron_cutoff,
                        MevEnergy                     gamma_cutoff);
};

//---------------------------------------------------------------------------//
} // namespace celeritas
