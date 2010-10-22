// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test matrix inverter"

#include <boost/test/unit_test.hpp>

#include <Eigen/SVD>

#include "Common/Log.hpp"

#include "Math/MatrixTypes.hpp"

using namespace std;
using namespace boost;
using namespace CF;
using namespace CF::Common;

using namespace Eigen;

////////////////////////////////////////////////////////////////////////////////

struct MatrixInverter_Fixture
{
  /// common setup for each test case
  MatrixInverter_Fixture()
  {
     // uncomment if you want to use arguments to the test executable
     //int*    argc = &boost::unit_test::framework::master_test_suite().argc;
     //char*** argv = &boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~MatrixInverter_Fixture()
  {
  }

  /// possibly common functions used on the tests below

  /// common values accessed by all tests goes here

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( MatrixInverter_TestSuite, MatrixInverter_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( SVDInverterTest )
{
  //CFinfo << "testing SVDMatrixInverter " << CFflush;
  // Exact solution is taken from http://www.mathematics-online.org/inhalt/beispiel/beispiel565/
  // System Matrix
  RealMatrix A(4,3);
  A << 2., -4., 5.,
       6.,  0., 3.,
       2., -4., 5.,
       6.,  0., 3.;

  // Right Hand Side
  RealVector4 b(1., 3., -1., 3.);
  
  // Known correct solution
  RealVector3 xCheck(0.5, 0.25, 0.);
  RealVector3 tol(1., 1., 1.); // 1% tolerance

  JacobiSVD<RealMatrix> svd(A);
//   RealVector3 x = svd.solve(b); // TODO: Solve method is in Eigen mercurial, but not the latest beta
// 
//   // Check if solution matches
//   for (Uint i=0; i<x.size(); i++) {
//     BOOST_CHECK_CLOSE(x[i]+1.,xCheck[i]+1.,tol[i]);
//   }
}


////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

