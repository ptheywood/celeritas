//----------------------------------*-C++-*----------------------------------//
// Copyright 2020-2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file testdetail/ParallelHandler.cc
//---------------------------------------------------------------------------//
#include "ParallelHandler.hh"

#include "corecel/io/ColorUtils.hh"
#include "corecel/sys/MpiCommunicator.hh"
#include "corecel/sys/MpiOperations.hh"

namespace celeritas
{
namespace testdetail
{
//---------------------------------------------------------------------------//
/*!
 * Construct with MPI communicator
 */
ParallelHandler::ParallelHandler(const MpiCommunicator& comm) : comm_(comm) {}

//---------------------------------------------------------------------------//
/*!
 * Print useful information at the beginning of the program
 */
void ParallelHandler::OnTestProgramStart(const ::testing::UnitTest&)
{
    if (comm_.rank() == 0)
    {
        std::cout << color_code('x') << "Testing "
#if CELERITAS_USE_MPI
                  << "on " << comm_.size() << " process"
                  << (comm_.size() > 1 ? "es" : "")
#else
                  << "in serial"
#endif
                  << color_code(' ') << std::endl;
    }
}

//---------------------------------------------------------------------------//
/*!
 * Print useful information at the end of the program
 */
void ParallelHandler::OnTestProgramEnd(const ::testing::UnitTest&) {}

//---------------------------------------------------------------------------//
/*!
 * Barrier at the beginning of each test
 */
void ParallelHandler::OnTestStart(const ::testing::TestInfo&)
{
    barrier(comm_);
}

//---------------------------------------------------------------------------//
/*!
 * Barrier at the end of each test
 */
void ParallelHandler::OnTestEnd(const ::testing::TestInfo&)
{
    std::cout << std::flush;
    barrier(comm_);
}

//---------------------------------------------------------------------------//
} // namespace testdetail
} // namespace celeritas
