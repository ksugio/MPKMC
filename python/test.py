import MPKMC

def calcEnergy(kmc, types):
    energy = [-52.3, -55.9, -59.6, -63.3, -67.0, -70.7, -74.4, -78.1, -81.7, -85.4, -89.1, -92.8, -96.4, -100.0]
    count = 0
    for tp in types:
        if tp == 14:
            count = count + 1
    #print count, types, energy[count]
    return energy[count] / kmc.ncol

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
    kmc.set_solvent(13)
    kmc.rand_seed = 12345
    kmc.add_solute_random(3, 14, 1)
    kmc.write('test.mpkmc', 0)
    #for i in range(64):
    #    x, y, z = kmc.index2grid(i)
    #    print kmc.grid2index(x, y, z), '(', x, y, z, ')'
    #print kmc.cluster_indexes(17)
    #types = (13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13)
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
    #for i in range(kmc.ntot):
    #    ene, update = kmc.calc_energy(i, calcEnergy)
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

