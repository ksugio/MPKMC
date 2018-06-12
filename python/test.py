import MPKMC

def FSFCCEnergy(kmc, types):
    fsfcc = MPKMC.fsfcc()
    fsfcc.set_parm(types[0])
    return fsfcc.energy(kmc, types)

if __name__ == "__main__":
    #etbfile = 'Al-Si.etb'
    #Kb = 86.1735e-6; # ev/K
    #Ry = 13.6058; # ev
    #Kbry = Kb*Ry
    #T = 1.0;
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5),\
        (0.5, 0, -0.5))
    kmc = MPKMC.new(4, 2, 2, 2, 13, 1000, 1000, 1000)
    kmc.set_unitcell(uc)
    kmc.set_cluster(clu)
    kmc.calc_rot_index(5.0)
    kmc.set_solvent(29)
    kmc.rand_seed = 12345
    #kmc.add_solute_random(3, 14, 1)
    #kmc.write('test.mpkmc', 0)
    #print kmc.search_cluster(types)
    #kmc.read_table(etbfile)
    #kmc.write_table('Al-Si3.etb')
    #kmc.set_matrix(13)
    #kmc.add_diffuse_random(3, 14)
    #for ind in range(kmc.ntot):
    #    ids = kmc.cluster_indexes(ind)
    #    print ind, kmc.search_cluster_ids(ids)
    #kmc.htable = 'test header'
    #print kmc.htable
    #kmc.htable = 'abcdefg'
    #print kmc.htable
    #for i in range(kmc.ntable):
    #    print kmc.get_table(i)
    for i in range(kmc.ntot):
        ene, update = kmc.calc_energy(i, FSFCCEnergy)
        print i, ene, update
    #    tp, pos, ene = kmc.grid_data(i)
    #    if tp == 14:
    #        print i, tp, pos, ene
    #print '-----'
    #for i in range(10000):
    #    ret, update = kmc.jump(Kbry*T, calcEnergy)
    #for i in range(kmc.ntot):
    #    tp, pos, ene = kmc.grid_data(i)
    #    if tp == 14:
    #        print i, tp, pos, ene
    #kmc.write_table('test.etb')

