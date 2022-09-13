import MPKMC
import numpy as np

Meam = MPKMC.meam()

def MEAMEnergy(kmc, types):
  return Meam.energy(kmc, types)

if __name__ == "__main__":
    Ntry = 100000
    T = range(800, 600, -10)
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (13, 13, 13, 13)
    pv = ((4.04466, 0, 0), (0, 4.04466, 0), (0, 0, 4.04466))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5)) 
    kmc = MPKMC.new(4, 10, 10, 10, 13)
    meam = MPKMC.meam()
    kmc.kb = 86.1735e-6; # ev/K
    kmc.freq = 1.0e13
    kmc.mige = 0.66 # vacancy migration energy in Aluminum
    kmc.rand_seed = 123456
    kmc.table_use = False
    kmc.event_record = True
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    kmc.add_solute_random(20, 0, 1)
    #kmc.grid_energy(MEAMEnergy)
    meam.grid_energy(kmc)
    print('(totmcs, temp, ntry, njump, table_update, ntable, tote, tott, time) ngroup')
    for temp in T:
      #ret = kmc.jump(Ntry, temp, MEAMEnergy)
      ret = meam.jump(kmc, Ntry, temp)
      print(ret, kmc.find_solute_group(0.71))
    kmc.write('meamAl.mpkmc', 8)





