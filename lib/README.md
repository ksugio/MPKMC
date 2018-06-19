# Compile in Windows
Visual Studio Community, Python library and Zlib are required to create MPKMC.dll (MPKMC.pyd).
Open MPKMC.sln and set include path and library path according to [Property Manager] - [Microsoft.cpp.Win32.user] - [VC++ Directory] - [Include Directory] and [Library Directory].
Example of include path is shown.

    C:\Python27\include
    C:\Python27\Lib\site-packages\numpy\core\include
    C:\lib\zlib128-dll\include

Example of library path is shown.

    C:\Python27\libs
    C:\Python27\Lib\site-packages\numpy\core\lib
    C:\lib\zlib128-dll\lib

Build in Release mode and MPKMC.pyd is copied to python directory.

# Compile in Linux
Edit Makefile and execute make.

    vi Makefile
    make install

MPKMC.so is created and copied to python directory.

# References
+ CLASSES
  + new(nuc, nx, ny, nz, ncluster, nsolute_max, ntable_step, nevent_step) : create new kmc data
  + read(fname, [version=1]) : read kmc data from file
    + CLASS METHODS
      + add_cluster(types) : add cluster in table and return table index
      + add_cluster_ids(ids) : add cluster in table and return table index
      + add_rot_index(ids) : add rotation index
      + add_solute(id, type, jump) : add a solute atom by id
      + add_solute_random(num, type, jump) : add solute atoms randomly
      + calc_energy(id, func) : calculate energy
      + calc_rot_index(step, tol) : calculate rotation index
      + cluster_indexes(id) : return cluster indexes
      + energy_history(ehist) : setup energy history
      + grid2index(p, x, y, z) : return index from grid position
      + grid_item(id) : return grid item
      + index2grid(id) : return grid position from index
      + jump(ntry, kt, func) : jump diffusions by KMC method
      + read_table(filename) : read energy table
      + real_pos(cp) : return real position
      + reset_table() : reset reference count of energy table
      + search_cluster(types) : search cluster in table and return table index
      + search_cluster_ids(ids) : search cluster in table and return table index
      + set_cluster(cluster) : set atom position of cluster
      + set_unitcell(uc, types, pv) : set unit cell
      + solute_item(id) : return solute item
      + sort_table() : sort energy table by reference count
      + step_backward(count) : take steps backward
      + step_forward(count) : take steps forward
      + step_go(step) : go to step
      + table_item(id) : return energy table item
      + total_energy(func) : calculate total energy
      + write(filename, comp) : write kmc data
      + write_table(filename) : write energy table
    + CLASS DATA
      + cluster : atom positions of cluster
      + htable : header of table
      + ncluster : number of atoms in cluster
      + nevent : number of events
      + nrot : number of rotation index
      + nsolute : number of solute atoms
      + nsolute_max : maximum number of solute atoms
      + ntable : number of table
      + ntot : total number of atom sites
      + nuc : number of atoms in unit cell
      + pv : primitive vector
      + rand_seed : seed of random number
      + rcluster : real atom positions of cluster
      + size : size of simulation cell
      + step : current step
      + table_use : flag for using table
      + tote : total energy
      + uc : atom positions of unit cell
      + uc_types : atom types of unit cell
  + fsfcc()
    + CLASS METHODS
      + energy(kmc, types) : calculate cluster energy
    + CLASS DATA
      + lc : lattice constant
      + type : atom type
