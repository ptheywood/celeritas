//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/GlobalGeoTestBase.hh
//---------------------------------------------------------------------------//
#pragma once

#include "GlobalTestBase.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
/*!
 * Reuse geometry across individual tests.
 *
 * This is helpful for slow geometry construction or if the geometry has
 * trouble building/destroying multiple times per execution due to global
 * variable usage (VecGeom, Geant4).
 *
 * The "geometry basename" should be the filename without extension of a
 * geometry file inside `test/celeritas/data`.
 */
class GlobalGeoTestBase : virtual public GlobalTestBase
{
  public:
    // Overload with the base filename of the geometry
    virtual const char* geometry_basename() const = 0;

    // Construct a geometry that's persistent across tests
    SPConstGeo build_geometry() override;

    // Clear the lazy geometry
    static void reset_geometry();

  private:
    //// LAZY GEOMETRY CONSTRUCTION AND CLEANUP FOR VECGEOM ////

    struct LazyGeo
    {
        std::string basename{};
        SPConstGeo  geo{};
    };

    static LazyGeo& lazy_geo();

    class CleanupGeoEnvironment : public ::testing::Environment
    {
        void SetUp() override {}
        void TearDown() override { GlobalGeoTestBase::reset_geometry(); }
    };
};

//---------------------------------------------------------------------------//
} // namespace test
} // namespace celeritas
