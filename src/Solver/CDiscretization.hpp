// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CDiscretization_hpp
#define CF_Solver_CDiscretization_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Solver/CMethod.hpp"
#include "Solver/LibSolver.hpp"

namespace CF {
  namespace Mesh { class CMesh; }
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API CDiscretization : public Solver::CMethod {

public: // typedefs

  /// type of pointer to Component
  typedef boost::shared_ptr<CDiscretization> Ptr;
  /// type of pointer to constant Component
  typedef boost::shared_ptr<CDiscretization const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CDiscretization ( const std::string& name );

  /// Virtual destructor
  virtual ~CDiscretization();

  /// Get the class name
  static std::string type_name () { return "CDiscretization"; }

  virtual void setup() {}

  // functions specific to the CDiscretization component
  
  virtual void compute_rhs() = 0;
  
protected: // data

  boost::weak_ptr<Mesh::CMesh> m_mesh;
  
};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CDiscretization_hpp
