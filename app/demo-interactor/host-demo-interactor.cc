//----------------------------------*-C++-*----------------------------------//
// Copyright 2020-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file demo-interactor/host-demo-interactor.cc
//---------------------------------------------------------------------------//

#include <cstddef>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "celeritas_version.h"
#include "corecel/io/Logger.hh"
#include "corecel/sys/DeviceIO.json.hh"
#include "corecel/sys/KernelRegistryIO.json.hh"
#include "corecel/sys/MpiCommunicator.hh"
#include "corecel/sys/ScopedMpiInit.hh"
#include "celeritas/phys/ParticleParams.hh"

#include "HostKNDemoRunner.hh"
#include "KNDemoIO.hh"
#include "LoadXs.hh"

using namespace celeritas;
using namespace demo_interactor;
using std::cerr;
using std::cout;
using std::endl;

namespace demo_interactor
{
//---------------------------------------------------------------------------//
/*!
 * Construct particle parameters.
 */
std::shared_ptr<ParticleParams> load_params()
{
    using namespace celeritas::units;
    constexpr auto zero   = zero_quantity();
    constexpr auto stable = ParticleRecord::stable_decay_constant();

    return std::make_shared<ParticleParams>(
        ParticleParams::Input{{"electron",
                               pdg::electron(),
                               MevMass{0.5109989461},
                               ElementaryCharge{-1},
                               stable},
                              {"gamma", pdg::gamma(), zero, zero, stable}});
}

//---------------------------------------------------------------------------//
/*!
 * Run, launch, and output.
 */
void run(std::istream& is)
{
    // Read input options
    auto inp = nlohmann::json::parse(is);

    // Construct runner
    HostKNDemoRunner run(load_params(), demo_interactor::load_xs());

    // For now, only do a single run
    auto run_args = inp.at("run").get<demo_interactor::KNDemoRunArgs>();
    CELER_EXPECT(run_args.energy > 0);
    CELER_EXPECT(run_args.num_tracks > 0);
    CELER_EXPECT(run_args.max_steps > 0);
    auto result = run(run_args);

    nlohmann::json outp = {
        {"run", run_args},
        {"result", result},
        {
            "runtime",
            {
                {"version", std::string(celeritas_version)},
                {"device", celeritas::device()},
                {"kernels", celeritas::kernel_registry()},
            },
        },
    };
    cout << outp.dump() << endl;
}
} // namespace demo_interactor

//---------------------------------------------------------------------------//
/*!
 * Execute and run.
 */
int main(int argc, char* argv[])
{
    ScopedMpiInit scoped_mpi(&argc, &argv);
    if (ScopedMpiInit::status() == ScopedMpiInit::Status::initialized
        && MpiCommunicator::comm_world().size() > 1)
    {
        CELER_LOG(critical) << "This app cannot run in parallel";
        return EXIT_FAILURE;
    }

    // Process input arguments
    std::vector<std::string> args(argv, argv + argc);
    if (args.size() != 2 || args[1] == "--help" || args[1] == "-h")
    {
        cerr << "usage: " << args[0] << " {input}.json" << endl;
        return EXIT_FAILURE;
    }

    if (args[1] != "-")
    {
        std::ifstream infile(args[1]);
        if (!infile)
        {
            CELER_LOG(critical) << "Failed to open '" << args[1] << "'";
            return EXIT_FAILURE;
        }
        run(infile);
    }
    else
    {
        // Read input from STDIN
        run(std::cin);
    }

    return EXIT_SUCCESS;
}
