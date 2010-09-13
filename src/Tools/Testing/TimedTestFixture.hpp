#ifndef CF_Tools_Tests_Timer_hpp
#define CF_Tools_Tests_Timer_hpp

#include <boost/test/framework.hpp>
#include <boost/timer.hpp>

#include <boost/mpi/timer.hpp>

#include "Common/Log.hpp"
#include "Common/MPI/PEInterface.hpp"

using namespace CF;
using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Tools {
namespace Testing {

////////////////////////////////////////////////////////////////////////////////

/// Any test using this fixture (or a derivative) will be timed
class TimedTestFixture {
public:

  TimedTestFixture() {
    test_unit_start(boost::unit_test::framework::current_test_case());
  }

  ~TimedTestFixture() {
    test_unit_finish(boost::unit_test::framework::current_test_case());
  }

  /// Start timing when a test starts
  void test_unit_start( boost::unit_test::test_unit const& unit) {
    m_timer.restart();
    if(PEInterface::instance().is_init())
      m_mpi_timer.restart();
  }

  /// Stop timing when a test ends
  void test_unit_finish( boost::unit_test::test_unit const& unit ) {
    if(PEInterface::instance().rank() > 0)
      return;
    // TODO: Provide more generic support for output in CDash format
    std::cout << "<DartMeasurement name=\"" << unit.p_name.get() << " time\" type=\"numeric/double\">" << (PEInterface::instance().is_init() ? m_mpi_timer.elapsed() : m_timer.elapsed()) << "</DartMeasurement>" << std::endl;
  }
private:
  boost::timer m_timer;
  boost::mpi::timer m_mpi_timer;
};

////////////////////////////////////////////////////////////////////////////////

} // Testing
} // Tools
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Tools_Tests_TimedTestFixture_hpp
