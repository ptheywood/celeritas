//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/GeantTestBase.cc
//---------------------------------------------------------------------------//
#include "GeantTestBase.hh"

#include <string>

#include "celeritas_cmake_strings.h"
#include "celeritas/em/FluctuationParams.hh"
#include "celeritas/em/process/BremsstrahlungProcess.hh"
#include "celeritas/em/process/ComptonProcess.hh"
#include "celeritas/em/process/EIonizationProcess.hh"
#include "celeritas/em/process/EPlusAnnihilationProcess.hh"
#include "celeritas/em/process/GammaConversionProcess.hh"
#include "celeritas/em/process/MultipleScatteringProcess.hh"
#include "celeritas/em/process/PhotoelectricProcess.hh"
#include "celeritas/ext/GeantImporter.hh"
#include "celeritas/ext/GeantSetup.hh"
#include "celeritas/geo/GeoMaterialParams.hh"
#include "celeritas/geo/GeoParams.hh"
#include "celeritas/global/ActionRegistry.hh"
#include "celeritas/global/alongstep/AlongStepGeneralLinearAction.hh"
#include "celeritas/io/ImportData.hh"
#include "celeritas/io/ImportedElementalMapLoader.hh"
#include "celeritas/mat/MaterialParams.hh"
#include "celeritas/phys/CutoffParams.hh"
#include "celeritas/phys/ImportedProcessAdapter.hh"
#include "celeritas/phys/ParticleParams.hh"
#include "celeritas/phys/PhysicsParams.hh"
#include "celeritas/random/RngParams.hh"
#include "celeritas/track/TrackInitParams.hh"

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
namespace
{
//---------------------------------------------------------------------------//
std::string gdml_filename(const char* basename)
{
    return std::string(basename) + std::string(".gdml");
}

//---------------------------------------------------------------------------//
ImportData load_import_data(std::string filename)
{
    GeantPhysicsOptions options;
    options.em_bins_per_decade = 14;
    // XXX static for now so that Vecgeom or other clients can access geant4
    // before the test explodes. Instead let's make this a class static
    // variable that can be reset maybe? Probably doesn't matter in practice
    // because geant still can't handle multiple runs per exe.
    static GeantImporter import(GeantSetup{filename, options});
    return import();
}

//---------------------------------------------------------------------------//
//! Test for equality of two C strings
bool cstring_equal(const char* lhs, const char* rhs)
{
    return std::strcmp(lhs, rhs) == 0;
}

//---------------------------------------------------------------------------//
} // namespace

//---------------------------------------------------------------------------//
//! Whether Geant4 dependencies match those on the CI build
bool GeantTestBase::is_ci_build()
{
    return cstring_equal(celeritas_rng, "XORWOW")
           && cstring_equal(celeritas_clhep_version, "2.4.6.0")
           && cstring_equal(celeritas_geant4_version, "11.0.3");
}

//---------------------------------------------------------------------------//
//! Whether Geant4 dependencies match those on Wildstyle
bool GeantTestBase::is_wildstyle_build()
{
    return cstring_equal(celeritas_rng, "XORWOW")
           && cstring_equal(celeritas_clhep_version, "2.4.6.0")
           && cstring_equal(celeritas_geant4_version, "11.0.3");
}

//---------------------------------------------------------------------------//
//! Whether Geant4 dependencies match those on Summit
bool GeantTestBase::is_summit_build()
{
    return cstring_equal(celeritas_rng, "XORWOW")
           && cstring_equal(celeritas_clhep_version, "2.4.5.1")
           && cstring_equal(celeritas_geant4_version, "11.0.0");
}

//---------------------------------------------------------------------------//
//! Get the Geant4 top-level geometry element (immutable)
const G4VPhysicalVolume* GeantTestBase::get_world_volume() const
{
    return GeantImporter::get_world_volume();
}

//---------------------------------------------------------------------------//
//! Get the Geant4 top-level geometry element
const G4VPhysicalVolume* GeantTestBase::get_world_volume()
{
    // Load geometry
    this->imported_data();
    return const_cast<const GeantTestBase*>(this)->get_world_volume();
}

//---------------------------------------------------------------------------//
// PROTECTED MEMBER FUNCTIONS
//---------------------------------------------------------------------------//
auto GeantTestBase::build_material() -> SPConstMaterial
{
    return MaterialParams::from_import(this->imported_data());
}

//---------------------------------------------------------------------------//
auto GeantTestBase::build_geomaterial() -> SPConstGeoMaterial
{
    return GeoMaterialParams::from_import(
        this->imported_data(), this->geometry(), this->material());
}

//---------------------------------------------------------------------------//
auto GeantTestBase::build_particle() -> SPConstParticle
{
    return ParticleParams::from_import(this->imported_data());
}

//---------------------------------------------------------------------------//
auto GeantTestBase::build_cutoff() -> SPConstCutoff
{
    return CutoffParams::from_import(
        this->imported_data(), this->particle(), this->material());
}

//---------------------------------------------------------------------------//
auto GeantTestBase::build_physics() -> SPConstPhysics
{
    PhysicsParams::Input input;
    input.materials      = this->material();
    input.particles      = this->particle();
    input.options        = this->build_physics_options();
    input.action_registry = this->action_reg().get();

    BremsstrahlungProcess::Options brem_options;
    brem_options.combined_model  = this->combined_brems();
    brem_options.enable_lpm      = true;
    brem_options.use_integral_xs = true;

    GammaConversionProcess::Options conv_options;
    conv_options.enable_lpm = true;

    EPlusAnnihilationProcess::Options epgg_options;
    epgg_options.use_integral_xs = true;

    EIonizationProcess::Options ioni_options;
    ioni_options.use_integral_xs = true;

    auto process_data
        = std::make_shared<ImportedProcesses>(this->imported_data().processes);
    input.processes.push_back(
        std::make_shared<ComptonProcess>(input.particles, process_data));
    input.processes.push_back(std::make_shared<PhotoelectricProcess>(
        input.particles,
        input.materials,
        process_data,
        make_imported_element_loader(this->imported_data().livermore_pe_data)));
    input.processes.push_back(std::make_shared<GammaConversionProcess>(
        input.particles, process_data, conv_options));
    input.processes.push_back(std::make_shared<EPlusAnnihilationProcess>(
        input.particles, epgg_options));
    input.processes.push_back(std::make_shared<EIonizationProcess>(
        input.particles, process_data, ioni_options));
    input.processes.push_back(std::make_shared<BremsstrahlungProcess>(
        input.particles,
        input.materials,
        process_data,
        make_imported_element_loader(this->imported_data().sb_data),
        brem_options));
    if (this->enable_msc())
    {
        input.processes.push_back(std::make_shared<MultipleScatteringProcess>(
            input.particles, input.materials, process_data));
    }
    return std::make_shared<PhysicsParams>(std::move(input));
}

//---------------------------------------------------------------------------//
auto GeantTestBase::build_init() -> SPConstTrackInit
{
    TrackInitParams::Input input;
    input.capacity   = 4096;
    input.max_events = 4096;
    return std::make_shared<TrackInitParams>(input);
}

//---------------------------------------------------------------------------//
auto GeantTestBase::build_along_step() -> SPConstAction
{
    auto& action_reg = *this->action_reg();
    auto  result     = AlongStepGeneralLinearAction::from_params(
        action_reg.next_id(),
        *this->material(),
        *this->particle(),
        *this->physics(),
        this->enable_fluctuation());
    CELER_ASSERT(result);
    CELER_ASSERT(result->has_fluct() == this->enable_fluctuation());
    CELER_ASSERT(result->has_msc() == this->enable_msc());
    action_reg.insert(result);
    return result;
}

//---------------------------------------------------------------------------//
auto GeantTestBase::build_physics_options() const -> PhysicsOptions
{
    PhysicsOptions options;
    options.secondary_stack_factor = this->secondary_stack_factor();
    return options;
}

//---------------------------------------------------------------------------//
// Lazily set up and load geant4
auto GeantTestBase::imported_data() const -> const ImportData&
{
    static struct
    {
        ImportData  imported;
        std::string geometry_basename;
    } i;
    std::string cur_basename = this->geometry_basename();
    if (i.geometry_basename != cur_basename)
    {
        CELER_VALIDATE(i.geometry_basename.empty(),
                       << "Geant4 currently crashes on second G4RunManager "
                          "instantiation (see issue #462)");
        // Note: importing may crash if Geant4 has an error
        i.imported          = load_import_data(this->test_data_path(
            "celeritas", gdml_filename(cur_basename.c_str()).c_str()));
        // Save basename *after* load
        i.geometry_basename = cur_basename;
    }
    CELER_ENSURE(i.imported);
    return i.imported;
}

//---------------------------------------------------------------------------//
std::ostream& operator<<(std::ostream& os, const PrintableBuildConf&)
{
    os << "RNG=\"" << celeritas_rng << "\", CLHEP=\""
       << celeritas_clhep_version << "\", Geant4=\""
       << celeritas_geant4_version << '"';
    return os;
}

//---------------------------------------------------------------------------//
} // namespace test
} // namespace celeritas
