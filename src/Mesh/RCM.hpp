#ifndef CF_Mesh_RCM_hpp
#define CF_Mesh_RCM_hpp

#include "Mesh/ConnectivityTableCF2.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

  /// Class implements reverse Cuthee-McKill algorithm on ConncectivityTble
  /// @author Antonino Bonnani
  /// @author Tiago Quintino
  class Mesh_API RCM
  {

    /// Transforms a connectivity from Cell to Node to Node to Node
    static void transformCellNode2NodeNode ( const ConnectivityTableCF2<Uint>& cellnode, ConnectivityTableCF2<Uint>& nodenode );

    /// Applies the Reverse Cuthill-McKee algorithm to the graph of the connectivity passed
    /// @param new_id is a vector with the new id numbers
    static void renumber ( ConnectivityTableCF2<Uint>& tb, std::valarray <Uint>& new_id );

    /// reads the a cell to node connectivity from the file
    static int read_input ( const std::string& filename, ConnectivityTableCF2<Uint>& cellnode );

    /// prints the connectivity to a file
    static void print_table ( const std::string& filename, const ConnectivityTableCF2<Uint>& tb );

  }; // RCM

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_RCM_hpp
