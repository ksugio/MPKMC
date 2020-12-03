import MPKMC
import numpy as np

if __name__ == "__main__":
    Ntry = 10000
    T = range(1100, 1000, -10)
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (29, 29, 29, 29)
    pv = ((3.615, 0, 0), (0, 3.615, 0), (0, 0, 3.615))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5),\
        (0.5, 0, -0.5), (1.0, 0, 0), (-1.0, 0, 0), (0, 1.0, 0), (0, -1.0, 0),\
        (0, 0, 1.0), (0, 0, -1.0))
    kmc = MPKMC.new(4, 10, 10, 10, 19)
    fsfcc = MPKMC.fsfcc()
    kmc.kb = 86.1735e-6 # ev/K
    kmc.rand_seed = 12345
    kmc.event_record = True
    kmc.table_use = True
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    kmc.add_solute_random(30, 0, 1)
    fsfcc.grid_energy(kmc)
    print('(totmcs, temp, ntry, njump, table_update, ntable, tote, time) ngroup')
    for temp in T:
        ret = fsfcc.jump(kmc, Ntry, temp)
        print(ret, kmc.find_solute_group(0.71))
    kmc.write('fsfccCu.mpkmc', 8)

