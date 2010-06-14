#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/OptionT.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

////////////////////////////////////////////////////////////////////////////////

CMeshReader::CMeshReader ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CMeshReader::~CMeshReader()
{
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::defineConfigOptions(Common::OptionList& options)
{
  options.add< OptionT<std::string> >  ( "File",  "File to read" , "" );
  //options.add< Common::OptionT<std::string> >  ( "Mesh",  "Mesh to construct" , "" );
}

//////////////////////////////////////////////////////////////////////////////

void CMeshReader::read( XmlNode& node  )
{
  // Get the mesh component in the tree
  /// @todo[1]: wait for Tiago for functionality

  // Get the file path
  boost::filesystem::path file = option("File")->value<std::string>();

  // Call implementation
  /// @todo wait for todo[1]
  // read_from_to(file,mesh);

}

//////////////////////////////////////////////////////////////////////////////

CMesh::Ptr CMeshReader::create_mesh_from(boost::filesystem::path& file)
{
  // Create the mesh
  CMesh::Ptr mesh (new CMesh("mesh"));

  // Call implementation
  read_from_to(file,mesh);

  // return the mesh
  return mesh;
}

//////////////////////////////////////////////////////////////////////////////

CMeshReader::BufferMap
CMeshReader::create_leaf_regions_with_buffermap (CRegion::Ptr& parent_region,
                                                 std::vector<std::string>& etypes)
{
  // Create regions for each element type
  BufferMap buffermap;
  BOOST_FOREACH(std::string& etype, etypes)
  {
    CRegion::Ptr etype_region = parent_region->create_leaf_region(etype);
    // CFinfo << "create: " << etype_region->full_path().string() << "\n" << CFflush;
    buffermap[etype]=boost::shared_ptr<CTable::Buffer>
      (new CTable::Buffer(get_named_component_typed<CTable>(*etype_region, "table").create_buffer(8192)));
  }
  return buffermap;
}

////////////////////////////////////////////////////////////////////////////////

void CMeshReader::remove_empty_leaf_regions(const CRegion::Ptr& parent_region)
{
  // loop over regions
  BOOST_FOREACH(CRegion& region, recursive_range_typed<CRegion>(*parent_region))
  {
    // find the empty regions
    if ( range_typed<CRegion>(region).empty() &&
        get_named_component_typed<CTable>(region, "table").get_table().size() == 0 )
      {
        // no elements in connectivity table --> remove this region
        //CFinfo << "remove: " << region->full_path().string() << "\n" << CFflush;
        CRegion::Ptr removed = boost::dynamic_pointer_cast<CRegion>(region.get_parent()->remove_component(region.name()));
        removed.reset();
      }
  }
}


} // Mesh
} // CF
