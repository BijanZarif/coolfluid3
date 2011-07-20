// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"


#include "Mesh/CRegion.hpp"

#include "RDM/CellLoop.hpp"
#include "RDM/Schemes/CSysN.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CSysN, RDM::CellTerm, LibSchemes > CSysN_Builder;

////////////////////////////////////////////////////////////////////////////////

CSysN::CSysN ( const std::string& name ) : RDM::CellTerm(name)
{
  regist_typeinfo(this);
}

CSysN::~CSysN() {}

void CSysN::execute()
{
  ElementLoop& loop = access_element_loop( type_name() );

  // loop on all regions configured by the user

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    loop.select_region( region );

    // loop all elements of this region

    loop.execute();
  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
