import sys
import coolfluid as cf
import numpy as np

env = cf.Core.environment()

# Global confifuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

class TaylorGreen:
  Ua = 0.
  Va = 0.
  D = 0.5
  
  t = np.array([])
  max_error = np.array([])
  
  model = None
  solver = None
  mesh = None
  
  
  def __init__(self, dt, element):
    self.dt = dt
    self.element = element
  
  def create_mesh(self, segments):
    domain = self.model.Domain
    blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
    points = blocks.create_points(dimensions = 2, nb_points = 6)
    points[0]  = [-0.5, -0.5]
    points[1]  = [0.5, -0.5]
    points[2]  = [-0.5, 0.]
    points[3]  = [0.5, 0.]
    points[4]  = [-0.5,0.5]
    points[5]  = [0.5, 0.5]

    block_nodes = blocks.create_blocks(2)
    block_nodes[0] = [0, 1, 3, 2]
    block_nodes[1] = [2, 3, 5, 4]

    block_subdivs = blocks.create_block_subdivisions()
    block_subdivs[0] = [segments, segments/2]
    block_subdivs[1] = [segments, segments/2]

    gradings = blocks.create_block_gradings()
    gradings[0] = [1., 1., 1., 1.]
    gradings[1] = [1., 1., 1., 1.]

    left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
    left_patch[0] = [2, 0]
    left_patch[1] = [4, 2]

    bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
    bottom_patch[0] = [0, 1]

    top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
    top_patch[0] = [5, 4]

    right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
    right_patch[0] = [1, 3]
    right_patch[1] = [3, 5]
    
    blocks.partition_blocks(nb_partitions = cf.Core.nb_procs(), direction = 0)
    
    mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
    self.mesh = mesh
    
    blocks.create_mesh(mesh.uri())
    
    create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
    create_point_region.coordinates = [0., 0.]
    create_point_region.region_name = 'center'
    create_point_region.mesh = mesh
    create_point_region.execute()
    
    
    if self.element == 'triag':
      triangulator = domain.create_component('Triangulator', 'cf3.mesh.MeshTriangulator')
      triangulator.mesh = mesh
      triangulator.execute()
    
    partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
    partitioner.mesh = mesh

    link_horizontal = partitioner.create_link_periodic_nodes()
    link_horizontal.source_region = mesh.topology.right
    link_horizontal.destination_region = mesh.topology.left
    link_horizontal.translation_vector = [-1., 0.]

    link_vertical = partitioner.create_link_periodic_nodes()
    link_vertical.source_region = mesh.topology.top
    link_vertical.destination_region = mesh.topology.bottom
    link_vertical.translation_vector = [0., -1.]

    partitioner.execute()
    
    coords = self.mesh.geometry.coordinates
    self.sample_coords = range(len(coords))
    self.probe_points = []
    
    for i in range(len(coords)):
      (x,y) = coords[i]
      if x == 0 and y == 0:
        self.probe_points.append(i)
        
    print 'probe_points', self.probe_points, coords[self.probe_points[0]]
    
    return mesh
    
  def setup_ic(self, u_tag, p_tag):
    #initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
    ic_u = self.solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = u_tag)
    ic_u.variable_name = 'Velocity'
    ic_u.regions = [self.mesh.topology.interior.uri()]
    ic_u.value = ['{Ua} - cos(pi/{D}*x)*sin(pi/{D}*y)'.format(Ua = self.Ua, D = self.D), '{Va} + sin(pi/{D}*x)*cos(pi/{D}*y)'.format(Va = self.Va, D = self.D)]

    ic_p = self.solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = p_tag)
    ic_p.regions = [self.mesh.topology.interior.uri()]
    ic_p.variable_name = 'Pressure'
    ic_p.value = ['-0.25*(cos(2*pi/{D}*x) + cos(2*pi/{D}*y))'.format(D = self.D)]
    
    ic_u.execute()
    ic_p.execute()
    self.model.Domain.write_mesh(cf.URI('init.pvtu'))
  
  def setup_model(self):
    if self.model != None:
      self.model.delete_component()
    
    model = cf.Core.root().create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
    self.model = model
    domain = model.create_domain()
    physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
    self.solver = model.create_solver('cf3.UFEM.Solver')
    
    # Physical constants
    physics.density = 1.
    physics.dynamic_viscosity = 0.001
    physics.reference_velocity = 1.
    
    return self.solver

  def add_pressure_bc(self, bc):
    bc.regions = [self.mesh.topology.uri()]
    nu = self.model.NavierStokesPhysics.kinematic_viscosity
    bc.add_function_bc(region_name = 'center', variable_name = 'Pressure').value = ['-0.25 * (cos(2*pi/{D}*(x - {Ua}*(t+{dt}))) + cos(2*pi/{D}*(y - {Va}*(t+{dt})))) * exp(-4*{nu}*pi^2/{D}^2*(t+{dt})) '.format(D = self.D, nu = nu, Ua = self.Ua, Va = self.Va, dt = self.dt)]
  
  def setup_implicit(self, segments, Ua, Va, D, theta):
    self.Ua = Ua
    self.Va = Va
    self.D = D
    self.segments = segments
    
    self.modelname = 'implicit'
    
    solver = self.setup_model()
    ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
    ns_solver.options.theta = theta
    ns_solver.options.use_specializations = True
    self.theta = theta
    
    mesh = self.create_mesh(segments)
    ns_solver.regions = [mesh.topology.interior.uri()]
    
    self.add_pressure_bc(ns_solver.BoundaryConditions)
    
    lss = ns_solver.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
    lss.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
    # lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.solver_type = 'Mumps'
    #lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.create_parameter_list('Amesos Settings').add_parameter(name = 'MaxProcs', value=1)
    #lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.AmesosSettings.add_parameter(name = 'Redistribute', value=True)
    #lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.AmesosSettings.create_parameter_list('Mumps').add_parameter(name='Equilibrate', value = False)

    solver.create_fields()
    self.setup_ic('navier_stokes_solution', 'navier_stokes_solution')
    
  def setup_semi_implicit(self, segments, Ua, Va, D, theta):  
    self.Ua = Ua
    self.Va = Va
    self.D = D
    self.segments = segments
    
    self.modelname = 'semi'
    
    solver = self.setup_model()
    ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokesSemiImplicit')
    ns_solver.options.blocked_system = True
    ns_solver.options.theta = theta
    self.theta = theta
    
    lss = ns_solver.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosCrsMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
    
    mesh = self.create_mesh(segments)
    ns_solver.regions = [mesh.topology.interior.uri()]
    
    self.add_pressure_bc(ns_solver.BoundaryConditions)
    
    lss.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.solver_type = 'Mumps'
    #lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.create_parameter_list('Amesos Settings').add_parameter(name = 'MaxProcs', value=1)
    #lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.AmesosSettings.add_parameter(name = 'Redistribute', value=True)
    #lss.SolutionStrategy.Parameters.LinearSolverTypes.Amesos.AmesosSettings.create_parameter_list('Mumps').add_parameter(name='Equilibrate', value = False)

    solver.create_fields()
    self.setup_ic('navier_stokes_solution', 'navier_stokes_solution')
  
  def iterate(self, numsteps, save_interval = 1, process_interval = 1):
    
    tstep = self.dt
    
    if (numsteps % save_interval) != 0:
      raise RuntimeError('Number of time steps cannot be divided by save_interval')

    self.outfile = open('uv_error-{modelname}-{element}-{segments}-dt_{tstep}-theta_{theta}-P{rank}.txt'.format(modelname = self.modelname, element = self.element, segments = self.segments, tstep = tstep, theta = self.theta, rank = cf.Core.rank()), 'w', 1)
    self.outfile.write('# time (s), max u error, max v error, max p error')
    for i in range(len(self.probe_points)):
      self.outfile.write(', probe {probe} u error, probe {probe} v error, probe {probe} p error'.format(probe = i))
    self.outfile.write('\n')
    
    # Time setup
    time = self.model.create_time()
    time.time_step = tstep

    # Setup a time series write
    final_end_time = numsteps*tstep
    self.iteration = 0
    time.end_time = 0.
    
    # Resize the error array
    self.max_error = np.zeros((3, numsteps))
    self.t = np.zeros(numsteps)

    while time.current_time < final_end_time:
      time.end_time += tstep
      self.model.simulate()
      #self.model.Solver.TimeLoop.NavierStokesExplicit.InnerLoop.PressureSystem.LSS.print_system('pressure_system.plt')
      self.t[self.iteration] = time.current_time
      if self.iteration % process_interval == 0:
        self.check_result()
      if self.iteration % save_interval == 0:
        self.model.Domain.write_mesh(cf.URI('taylor-green-' +str(self.iteration) + '.pvtu'))
      self.iteration += 1
      if self.iteration == 1:
        self.model.Solver.options.disabled_actions = ['InitialConditions']

    self.outfile.close()

    # print timings
    self.model.print_timing_tree()


  def check_result(self):
    t = self.t[self.iteration]
    Ua = self.Ua
    Va = self.Va
    D = self.D
    
    nu = self.model.NavierStokesPhysics.kinematic_viscosity
    
    try:
      u_sol = self.mesh.geometry.navier_stokes_solution
      p_sol = self.mesh.geometry.navier_stokes_solution
      u_idx = 0
      v_idx = 1
      p_idx = 2
    except AttributeError:
      u_sol = self.mesh.geometry.navier_stokes_u_solution
      p_sol = self.mesh.geometry.navier_stokes_p_solution
      u_idx = 0
      v_idx = 1
      p_idx = 0
      
    coords = self.mesh.geometry.coordinates
    x_arr = np.zeros(len(coords))
    y_arr = np.zeros(len(coords))
    p_num = np.zeros(len(coords))
    u_num = np.zeros(len(coords))
    v_num = np.zeros(len(coords))
    p_th = np.zeros(len(coords))
    u_th = np.zeros(len(coords))
    v_th = np.zeros(len(coords))
    for i in self.sample_coords:
      (x, y) = (x_arr[i], y_arr[i]) = coords[i]
      (u_num[i], v_num[i], p_num[i]) = ( u_sol[i][u_idx], u_sol[i][v_idx], p_sol[i][p_idx] )
      u_th[i] = Ua - np.cos(np.pi/D*(x-Ua*t))*np.sin(np.pi/D*(y-Va*t))*np.exp(-2.*nu*np.pi**2/D**2*t)
      v_th[i] = Va + np.sin(np.pi/D*(x-Ua*t))*np.cos(np.pi/D*(y-Va*t))*np.exp(-2.*nu*np.pi**2/D**2*t)
      p_th[i] = -0.25 * (np.cos(2*np.pi/D*(x - Ua*t)) + np.cos(2*np.pi/D*(y - Va*t)))*np.exp(-4.*nu*np.pi**2/D**2*t)
      
    self.max_error[0, self.iteration] = np.max(np.abs(u_th - u_num))
    self.max_error[1, self.iteration] = np.max(np.abs(v_th - v_num))
    self.max_error[2, self.iteration] = np.max(np.abs(p_th - p_num))
    
    # np.savetxt('duv-{t}.txt'.format(t = t), (x_arr, y_arr, np.abs(u_th-u_num), np.abs(v_th-v_num)))
    
    self.outfile.write('{t},{u},{v},{p}'.format(t = t, u = self.max_error[0, self.iteration], v = self.max_error[1, self.iteration], p = self.max_error[2, self.iteration]))
    for i in self.probe_points:
      self.outfile.write(',{u},{v},{p}'.format(u = u_th[i] - u_num[i], v = v_th[i] - v_num[i], p = p_th[i] - p_num[i]))
    self.outfile.write('\n')


taylor_green = TaylorGreen(dt = 0.004, element='quad')
#taylor_green.setup_implicit(64, 0.3, 0.2, D=0.5, theta=0.5)
taylor_green.setup_semi_implicit(64, 0.3, 0.2, D=0.5, theta=0.5)
taylor_green.iterate(3000, 50, 1)

