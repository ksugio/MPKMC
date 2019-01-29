import MPKMC
import math as m

if __name__ == "__main__":
    uc = ((0, 0, 0), (2.0/3.0, 1.0/3.0, 0.5))
    uc_types = (22, 22)
    pv = ((1.0, 0, 0), (-0.5, 0.866025403, 0.0), (0, 0, 1.0)) 
    kmc = MPKMC.new(2, 4, 4, 4, 13, 1000, 1000, 1000)
    kmc.set_unitcell(uc, uc_types, pv)
    #print kmc.uc
    kmc.write('test2.mpkmc', 8)
    #kmc = MPKMC.read('Al-Cu_5.kmc', 0)
    #kmc.write('Al-Cu_5m2.kmc', 8)
    #uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    #uc_types = (13, 13, 13, 13)
    #pv = ((3.615, 0, 0), (0, 3.615, 0), (0, 0, 3.615))
    #clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
    #    (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
    #    (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5),\
    #    (0.5, 0, -0.5))
    #jclu = (1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1)
    #kmc = MPKMC.new(4, 10, 10, 10, 13, 100, 1000, 10000)
    #kmc.set_unitcell(uc, uc_types, pv)
    #kmc.set_cluster(clu, jclu)
    #kmc.calc_rot_index(5.0, 1.0e-6)
    #kmc.read_table('Al-Si.etb')
    #count, tb = kmc.search_table('p0t14,t14n3')
    #print count, kmc.ntable
    #print tb
