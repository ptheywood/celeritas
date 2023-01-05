//----------------------------------*-C++-*----------------------------------//
// Copyright 2022 UT-Battelle, LLC, and other Celeritas developers.
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: (Apache-2.0 OR MIT)
//---------------------------------------------------------------------------//
//! \file celeritas/user/StepCollector.test.cc
//---------------------------------------------------------------------------//
#include "celeritas/user/StepCollector.hh"

#include "corecel/cont/Span.hh"
#include "celeritas/global/ActionRegistry.hh"
#include "celeritas/global/Stepper.hh"
#include "celeritas/global/alongstep/AlongStepUniformMscAction.hh"
#include "celeritas/phys/PDGNumber.hh"
#include "celeritas/phys/ParticleParams.hh"
#include "celeritas/phys/Primary.hh"

#include "../SimpleTestBase.hh"
#include "../TestEm15Base.hh"
#include "../TestEm3Base.hh"
#include "CaloTestBase.hh"
#include "ExampleCalorimeters.hh"
#include "ExampleMctruth.hh"
#include "MctruthTestBase.hh"
#include "celeritas_test.hh"

using celeritas::units::MevEnergy;

namespace celeritas
{
namespace test
{
//---------------------------------------------------------------------------//
// TEST FIXTURES
//---------------------------------------------------------------------------//

class KnStepCollectorTestBase : public SimpleTestBase,
                                virtual public StepCollectorTestBase
{
  protected:
    VecPrimary make_primaries(size_type count) override
    {
        Primary p;
        p.particle_id = this->particle()->find(pdg::gamma());
        CELER_ASSERT(p.particle_id);
        p.energy    = MevEnergy{10.0};
        p.track_id  = TrackId{0};
        p.position  = {0, 0, 0};
        p.direction = {1, 0, 0};
        p.time      = 0;

        std::vector<Primary> result(count, p);
        for (auto i : range(count))
        {
            result[i].event_id = EventId{i};
        }
        return result;
    }
};

class KnMctruthTest : public KnStepCollectorTestBase, public MctruthTestBase
{
};

class KnCaloTest : public KnStepCollectorTestBase, public CaloTestBase
{
    VecString get_detector_names() const final { return {"inner"}; }
};

//---------------------------------------------------------------------------//

class TestEm3CollectorTestBase : public TestEm3Base,
                                 virtual public StepCollectorTestBase
{
    //! Use MSC
    bool enable_msc() const override { return true; }

    SPConstAction build_along_step() override
    {
        auto&              action_reg = *this->action_reg();
        UniformFieldParams field_params;
        field_params.field = {0, 0, 1 * units::tesla};
        auto result        = AlongStepUniformMscAction::from_params(
            action_reg.next_id(), *this->physics(), field_params);
        CELER_ASSERT(result);
        CELER_ASSERT(result->has_msc() == this->enable_msc());
        action_reg.insert(result);
        return result;
    }

    VecPrimary make_primaries(size_type count) override
    {
        Primary p;
        p.energy    = MevEnergy{10.0};
        p.position  = {-22, 0, 0};
        p.direction = {1, 0, 0};
        p.time      = 0;
        std::vector<Primary> result(count, p);

        auto electron = this->particle()->find(pdg::electron());
        CELER_ASSERT(electron);
        auto positron = this->particle()->find(pdg::positron());
        CELER_ASSERT(positron);

        for (auto i : range(count))
        {
            result[i].event_id    = EventId{0};
            result[i].track_id    = TrackId{i};
            result[i].particle_id = (i % 2 == 0 ? electron : positron);
        }
        return result;
    }
};

#define TestEm3MctruthTest TEST_IF_CELERITAS_GEANT(TestEm3MctruthTest)
class TestEm3MctruthTest : public TestEm3CollectorTestBase,
                           public MctruthTestBase
{
};

#define TestEm3CaloTest TEST_IF_CELERITAS_GEANT(TestEm3CaloTest)
class TestEm3CaloTest : public TestEm3CollectorTestBase, public CaloTestBase
{
    VecString get_detector_names() const final
    {
        return {"gap_lv_0", "gap_lv_1", "gap_lv_2"};
    }
};

//---------------------------------------------------------------------------//
// ERROR CHECKING
//---------------------------------------------------------------------------//

TEST_F(KnStepCollectorTestBase, mixing_types)
{
    auto calos = std::make_shared<ExampleCalorimeters>(
        *this->geometry(), std::vector<std::string>{"inner"});
    auto mctruth = std::make_shared<ExampleMctruth>();

    StepCollector::VecInterface interfaces = {calos, mctruth};

    EXPECT_THROW((StepCollector{std::move(interfaces),
                                this->geometry(),
                                this->action_reg().get()}),
                 celeritas::RuntimeError);
}

TEST_F(KnStepCollectorTestBase, multiple_interfaces)
{
    // Add mctruth twice so each step is doubly written
    auto                        mctruth = std::make_shared<ExampleMctruth>();
    StepCollector::VecInterface interfaces = {mctruth, mctruth};
    auto                        collector  = std::make_shared<StepCollector>(
        std::move(interfaces), this->geometry(), this->action_reg().get());

    // Do one step with two tracks
    {
        StepperInput step_inp;
        step_inp.params          = this->core();
        step_inp.num_track_slots = 2;

        Stepper<MemSpace::host> step(step_inp);

        auto primaries = this->make_primaries(2);
        step(make_span(primaries));
    }

    EXPECT_EQ(4, mctruth->steps().size());
}

//---------------------------------------------------------------------------//
// KLEIN-NISHINA
//---------------------------------------------------------------------------//

TEST_F(KnMctruthTest, single_step)
{
    auto result = this->run(8, 1);

    static const int expected_event[] = {0, 1, 2, 3, 4, 5, 6, 7};
    EXPECT_VEC_EQ(expected_event, result.event);
    static const int expected_track[] = {0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_VEC_EQ(expected_track, result.track);
    static const int expected_step[] = {1, 1, 1, 1, 1, 1, 1, 1};
    EXPECT_VEC_EQ(expected_step, result.step);
    if (!CELERITAS_USE_VECGEOM)
    {
        static const int expected_volume[] = {1, 1, 1, 1, 1, 1, 1, 1};
        EXPECT_VEC_EQ(expected_volume, result.volume);
    }
    static const double expected_pos[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                          0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_VEC_SOFT_EQ(expected_pos, result.pos);
    static const double expected_dir[] = {1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0,
                                          1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};
    EXPECT_VEC_SOFT_EQ(expected_dir, result.dir);
}

TEST_F(KnMctruthTest, two_step)
{
    auto result = this->run(4, 2);

    // clang-format off
    static const int expected_event[] = {0, 0, 1, 1, 2, 2, 3, 3};
    EXPECT_VEC_EQ(expected_event, result.event);
    static const int expected_track[] = {0, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_VEC_EQ(expected_track, result.track);
    static const int expected_step[] = {1, 2, 1, 2, 1, 2, 1, 2};
    EXPECT_VEC_EQ(expected_step, result.step);
    if (!CELERITAS_USE_VECGEOM)
    {
        static const int expected_volume[] = {1, 1, 1, 1, 1, 2, 1, 2};
        EXPECT_VEC_EQ(expected_volume, result.volume);
    }
    static const double expected_pos[] = {0, 0, 0, 2.6999255778482, 0, 0, 0, 0, 0, 3.5717683161497, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0, 0, 5, 0, 0};
    EXPECT_VEC_SOFT_EQ(expected_pos, result.pos);
    static const double expected_dir[] = {1, 0, 0, 0.45619379667222, 0.14402721708137, -0.87814769863479, 1, 0, 0, 0.8985574206844, -0.27508545475671, -0.34193940152356, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0};
    EXPECT_VEC_SOFT_EQ(expected_dir, result.dir);
    // clang-format on
}

TEST_F(KnCaloTest, single_event)
{
    auto result = this->run(1, 64);

    static const double expected_edep[] = {0.00043564799352598};
    EXPECT_VEC_SOFT_EQ(expected_edep, result.edep);
}

//---------------------------------------------------------------------------//
// TESTEM3
//---------------------------------------------------------------------------//

TEST_F(TestEm3MctruthTest, four_step)
{
    auto result = this->run(4, 4);

    if (this->is_ci_build() || this->is_summit_build()
        || this->is_wildstyle_build())
    {
        // clang-format off
        static const int expected_event[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        EXPECT_VEC_EQ(expected_event, result.event);
        static const int expected_track[] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
        EXPECT_VEC_EQ(expected_track, result.track);
        static const int expected_step[] = {1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4};
        EXPECT_VEC_EQ(expected_step, result.step);
        if (!CELERITAS_USE_VECGEOM)
        {
            static const int expected_volume[] = {1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2, 1, 2, 2, 2};
            EXPECT_VEC_EQ(expected_volume, result.volume);
        }
        static const double expected_pos[] = {-22, 0, 0, -20, 0.62729376699828, 0, -19.974880329316, 0.63919631534337, 0.0048226552156834, -19.93403368206, 0.64565991388053, 0.023957106651518, -22, 0, 0, -20, -0.6272937669977, 0, -19.968081477436, -0.64565253052318, 0.0081439674481249, -19.91982035106, -0.66229283729322, 0.0308848424967, -22, 0, 0, -20, 0.62729376699806, 0, -19.972026591268, 0.66425280181022, -0.0037681439073629, -19.982100207979, 0.68573542038722, 0.027933364396019, -22, 0, 0, -20, -0.62729376699777, 0, -19.969797686903, -0.66354024672438, -0.0032805361823643, -19.954139884858, -0.71455560351776, 0.0075436422799345};
        EXPECT_VEC_SOFT_EQ(expected_pos, result.pos);
        static const double expected_dir[] = {1, 0, 0, 0.82087264698347, 0.57111128288133, 0, 0.86898688645512, 0.46973495237486, 0.15559841158064, 0.93933572337412, 0.33065746542153, -0.091181354202613, 1, 0, 0, 0.8208726469842, -0.57111128288028, 0, 0.97042759391972, -0.23162277007504, 0.067979241993019, -0.049256190849848, -0.57458307380017, 0.81696274025522, 1, 0, 0, 0.82087264698375, 0.57111128288092, 0, -0.21515133985226, 0.77283313432545, 0.59702499733971, -0.48943328665481, 0.50499006011609, 0.71094310398106, 1, 0, 0, 0.8208726469841, -0.57111128288041, 0, 0.45731722153487, -0.78666386310603, 0.41475405407388, -0.062196556823769, -0.95423613503652, -0.29251493450733};
        EXPECT_VEC_SOFT_EQ(expected_dir, result.dir);
        // clang-format on
    }
    else
    {
        cout << "No output saved for combination of "
             << test::PrintableBuildConf{} << std::endl;
        result.print_expected();
    }
}

TEST_F(TestEm3CaloTest, thirtytwo_step)
{
    auto result = this->run(256, 32);

    static const double expected_edep[]
        = {1535.4185205798, 109.69434829612, 20.443067191226};
    EXPECT_VEC_NEAR(expected_edep, result.edep, 0.5);
}

//---------------------------------------------------------------------------//
} // namespace test
} // namespace celeritas