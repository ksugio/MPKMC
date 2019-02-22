import MPKMC
import numpy as np

def MEAMEnergy(kmc, types):
  meam = MPKMC.meam()
  return meam.energy(kmc, types)

if __name__ == "__main__":
    Ntry = 10000
    T = range(800, 600, -10)
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (13, 13, 13, 13)
    pv = ((4.04466, 0, 0), (0, 4.04466, 0), (0, 0, 4.04466))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5)) 
    kmc = MPKMC.new(4, 10, 10, 10, 13)
    kmc.kb = 86.1735e-6; # ev/K
    kmc.rand_seed = 12345
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    kmc.add_solute_random(20, 0, 1)
    kmc.grid_energy(MEAMEnergy)
    print '(totmcs, temp, ntry, njump, table_update, ntable, tote, time) ngroup'
    for temp in T:
      ret = kmc.jump(Ntry, temp, MEAMEnergy)
      print ret, kmc.find_solute_group(0.71)
    kmc.write('meamAl.mpkmc', 8)





