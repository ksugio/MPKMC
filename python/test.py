import MPKMC
import math as m

if __name__ == "__main__":
    #uc = ((0, 0, 0), (2.0/3.0, 1.0/3.0, 0.5))
    #uc_types = (22, 22)
    #pv = ((1.0, 0, 0), (-0.5, 0.866025403, 0.0), (0, 0, 1.0)) 
    #kmc = MPKMC.new(2, 4, 4, 4, 13, 1000, 1000, 1000)
    #kmc.set_unitcell(uc, uc_types, pv)
    #print kmc.uc
    #kmc.write('test2.mpkmc', 8)
    #kmc = MPKMC.read('Al-Cu_5.kmc', 0)
    #kmc.write('Al-Cu_5m2.kmc', 8)
    types = (13, 14, 14, 0, 15, 12, 6, 16, 8, 1)
    s = MPKMC.types2string(types)
    print s
    print MPKMC.string2types(s)
