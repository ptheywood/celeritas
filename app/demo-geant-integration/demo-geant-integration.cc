//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file demo-geant-integration/demo-geant-integration.cc
//---------------------------------------------------------------------------//

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <CLHEP/Random/Random.h>
#include <G4RunManager.hh>
#include <G4UImanager.hh>

#include "celeritas/ext/GeantConfig.hh"
#if !CELERITAS_G4_V10
#    include <G4RunManagerFactory.hh>
#else
#    include <G4MTRunManager.hh>
#endif

#if 0
#    include <FTFP_BERT.hh>
#else
#    include "celeritas/ext/GeantPhysicsOptions.hh"
#    include "celeritas/ext/detail/GeantPhysicsList.hh"
#endif

#include "corecel/Assert.hh"
#include "corecel/io/Logger.hh"
#include "corecel/sys/Environment.hh"
#include "corecel/sys/TypeDemangler.hh"
#include "accel/Logger.hh"

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"
#include "HepMC3Reader.hh"

namespace
{
//---------------------------------------------------------------------------//
void run(const std::string& macro_filename)
{
    // Set the random seed *before* the run manager is instantiated
    // (G4MTRunManager constructor uses the RNG)
    CLHEP::HepRandom::setTheSeed(0xcf39c1fa9a6e29bcul);

    std::unique_ptr<G4RunManager> run_manager;
#if CELERITAS_G4_V10
    if (CELERITAS_G4_MT)
    {
        run_manager = std::make_unique<G4MTRunManager>();
    }
    else
    {
        run_manager = std::make_unique<G4RunManager>();
    }
#else
    run_manager.reset(G4RunManagerFactory::CreateRunManager(
        CELERITAS_G4_MT ? G4RunManagerType::MT : G4RunManagerType::Serial));
#endif
    CELER_ASSERT(run_manager);
    celeritas::self_logger() = celeritas::make_mt_logger(*run_manager);
    CELER_LOG(info) << "Run manager type: "
                    << celeritas::TypeDemangler<G4RunManager>{}(*run_manager);

    // Construct geometry and SD factory
    run_manager->SetUserInitialization(new demo_geant::DetectorConstruction{});

#if 0
    // TODO: use full physics
    run_manager->SetUserInitialization(new FTFP_BERT);
#else
    // For now (reduced output) use just EM
    celeritas::GeantPhysicsOptions geant_phys_opts{};
    run_manager->SetUserInitialization(
        new celeritas::detail::GeantPhysicsList{geant_phys_opts});
#endif

    run_manager->SetUserInitialization(new demo_geant::ActionInitialization());

    G4UImanager* ui = G4UImanager::GetUIpointer();
    CELER_ASSERT(ui);
    CELER_LOG(status) << "Executing macro commands from '" << macro_filename
                      << "'";
    ui->ApplyCommand("/control/execute " + macro_filename);

    // Initialize run and process events
    run_manager->Initialize();
    run_manager->BeamOn(demo_geant::HepMC3Reader::instance()->num_events());
}

//---------------------------------------------------------------------------//
} // namespace

//---------------------------------------------------------------------------//
/*!
 * Execute and run.
 */
int main(int argc, char* argv[])
{
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() != 2 || args[1] == "--help" || args[1] == "-h")
    {
        std::cerr << "usage: " << args[0] << " {commands}.mac\n"
                  << "Environment variables:\n"
                  << "  G4FORCE_RUN_MANAGER_TYPE: MT or Serial\n"
                  << "  G4FORCENUMBEROFTHREADS: set CPU worker thread count\n"
                  << "  CELER_DISABLE_DEVICE: nonempty disables CUDA\n"
                  << "  CELER_LOG: global logging level\n"
                  << "  CELER_LOG_LOCAL: thread-local logging level\n"
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Run with threads and macro filename
    run(args[1]);

    CELER_LOG(status) << "Run completed successfully; exiting";
    return EXIT_SUCCESS;
}
