//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file demo-geant-integration/EventAction.hh
//---------------------------------------------------------------------------//
#pragma once

#include <G4UserEventAction.hh>

#include "accel/LocalTransporter.hh"

namespace demo_geant
{
//---------------------------------------------------------------------------//
/*!
 * Manage begin- and end-of-event setup.
 *
 * This class should be local to a thread/task/stream.
 */
class EventAction final : public G4UserEventAction
{
  public:
    //!@{
    //! \name Type aliases
    using SPTransporter = std::shared_ptr<celeritas::LocalTransporter>;
    //!@}

  public:
    explicit EventAction(SPTransporter transport);

    void BeginOfEventAction(const G4Event* event) final;
    void EndOfEventAction(const G4Event* event) final;

  private:
    SPTransporter transport_;
};

//---------------------------------------------------------------------------//
} // namespace demo_geant
