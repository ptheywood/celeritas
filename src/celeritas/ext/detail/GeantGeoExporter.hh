//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/ext/detail/GeantGeoExporter.hh
//---------------------------------------------------------------------------//
#pragma once

#include <string>

class G4VPhysicalVolume;

namespace celeritas
{
namespace detail
{
//---------------------------------------------------------------------------//
/*!
 * Export a Geant4 geometry to a GDML file.
 */
class GeantGeoExporter
{
  public:
    // Helper function to create a temporary file.
    static std::string make_tmpfile_name();

    // Construct from the world geometry pointer
    explicit GeantGeoExporter(const G4VPhysicalVolume* world);

    // Save to a file and return properties
    void operator()(const std::string& filename) const;

  private:
    const G4VPhysicalVolume* world_;
};

//---------------------------------------------------------------------------//
} // namespace detail
} // namespace celeritas
