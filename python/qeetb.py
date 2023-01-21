import MPKMC
import qetool
import os
import pandas as pd

def GenCell():
    Kb = 86.1735e-6; # ev/K
    Ry = 13.6058; # ev
    uc = ((0, 0, 0), (0.5, 0.5, 0.0), (0.5, 0.0, 0.5), (0.0, 0.5, 0.5))
    uc_types = (13, 13, 13, 13)
    pv = ((4.04360, 0, 0), (0, 4.04360, 0), (0, 0, 4.04360))
    clu = ((0, 0, 0), (0.5, 0.5, 0), (0, 0.5, -0.5), (-0.5, 0, -0.5),\
        (-0.5, 0.5, 0), (0, 0.5, 0.5), (0.5, 0, 0.5), (0.5, -0.5, 0),\
        (0, -0.5, 0.5), (-0.5, 0, 0.5), (-0.5, -0.5, 0), (0, -0.5, -0.5), (0.5, 0, -0.5))
    kmc = MPKMC.new(4, 2, 2, 2, 13, 100, 1000, 100)
    kmc.kb = Kb/Ry
    kmc.freq = 1.0e13 # frequency of atom
    kmc.mige = 0.66 # vacancy migration energy in Aluminum
    kmc.rand_seed = 12345
    kmc.set_unitcell(uc, uc_types, pv)
    kmc.set_cluster(clu, 13)
    kmc.calc_rot_index(5.0, 1.0e-6)
    return kmc

def genTable(etbfile, outdir, isolates, ebonds):
    kmc = GenCell()
    for outfile in qetool.listQEout(outdir):
        data = qetool.readQEout(outfile)
        clue = qetool.ClusterEnergy(data, isolates, ebonds)
        s = os.path.splitext(os.path.basename(outfile))[0]
        types = MPKMC.string2types(s)
        print(types, clue)
        kmc.add_cluster(types, clue, 0)
    kmc.write_table(etbfile)

def dummyQE(kmc, types):
    ebonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0 }
    tote = 0.0
    for type in types[1:]:
        combi = qetool.bondKey(types[0], type)
        tote += ebonds[combi]
    return tote / 2.0

def Compare(etbfile, outfile, isolates):
    kmc = GenCell()
    kmc.read_table(etbfile)
    data = qetool.readQEout(outfile)
    qetool.addSolute(kmc, data)
    kmc.grid_energy(dummyQE)
    qetote = qetool.BindingEnergy(data, isolates)
    kmctote = kmc.tote
    del kmc
    return outfile, qetote, kmctote, qetote-kmctote

#
# genarate .etb file from QE .out file
#
if __name__ == "__main__":
    etbfile = 'Al-Si-V.etb'
    outdir = 'AlSi60'
    isolates = { 0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    ebonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0 }
    genTable(etbfile, outdir, isolates, ebonds)
    # Compare 1
    outfiles = [ ('AlSi60/qAAAAAAAAAAAAA.out', (13, 13)),
                 ('AlSi60/qAAAAAASAAAAAA.out', (13, 14)),
                 ('AlSi60/qSASAAAAAAAAAA.out', (14, 14)),
                 ('AlSi60/qAA0AAAAAAAAAA.out', (0, 13)),
                 ('AlSi60/qAS0AAAAAAAAAA.out', (0, 14)) ]
    print(('File', 'QE', 'MC', 'Diff'))
    for outfile, combi in outfiles:
        print(Compare(etbfile, outfile, isolates))
    # Compare 2
    comp = []
    for outfile, types in qetool.listQEoutTypes('AlSi60'):
        if not 0 in types:
            comp.append(Compare(etbfile, outfile, isolates))
    df = pd.DataFrame(comp, columns=['File', 'QE', 'MC', 'Diff'])
    df.to_csv('Al-Si-V_cmp.csv', index=False)

