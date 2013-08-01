import sys
import coolfluid as cf

def copy_and_reset(source, domain):
  nb_items = len(source)
  destination = domain.create_component(source.name(), 'cf3.mesh.Field')
  destination.set_row_size(1)
  destination.resize(nb_items)

  for i in range(nb_items):
    destination[i][0] = source[i][0]
    source[i][0] = 0
    
  return destination

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('OriginalMesh','cf3.mesh.Mesh')


blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [16,16]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.partition_blocks(nb_partitions = cf.Core.nb_procs(), direction = 1)
blocks.create_mesh(mesh.uri())

link_horizontal = domain.create_component('LinkHorizontal', 'cf3.mesh.actions.LinkPeriodicNodes')
link_horizontal.mesh = mesh
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-1., 0.]
link_horizontal.execute()

make_par_data = root.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = mesh
make_par_data.execute()

# set time
time = domain.create_component('Time', 'cf3.solver.Time')
time.current_time = 2.
time.time_step = 0.2
time.iteration = 10

# Write a restart file containing the data generated by MakeParData
restart_file = cf.URI('restart-test.cf3restart')
writer = domain.create_component('Writer', 'cf3.solver.actions.WriteRestartFile')
writer.fields = [mesh.geometry.node_gids, mesh.elems_P0.element_gids]
writer.file = restart_file
writer.time = time
writer.execute()

# Store reference data and destroy the original
ref_node_gids = copy_and_reset(mesh.geometry.node_gids, domain)
ref_element_gids = copy_and_reset(mesh.elems_P0.element_gids, domain)
time.current_time = 0.
time.time_step = 1.
time.iteration = 0

# Read back the data
reader = domain.create_component('Reader', 'cf3.solver.actions.ReadRestartFile')
reader.mesh = mesh
reader.file = restart_file
reader.time = time
reader.execute()

# Check the result
differ = domain.create_component('Differ', 'cf3.common.ArrayDiff')
differ.left = ref_node_gids
differ.right = mesh.geometry.node_gids
differ.execute()
if not differ.properties()['arrays_equal']:
  raise Exception('Node GIDS do not match')

differ.left = ref_element_gids
differ.right = mesh.elems_P0.element_gids
differ.execute()
if not differ.properties()['arrays_equal']:
  raise Exception('Element GIDS do not match')

if time.current_time != 2. or time.time_step != 0.2 or time.iteration != 10:
  raise Exception('Error in time data')