import MPKMC
import qetool
import pandas as pd
import os

def dummyQE(kmc, types):
    ebonds = {(13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0}
    return qetool.bondClusterEnergy(types, ebonds)

#
# genarate .etb file from QE .out files with cluster energy
# if the cluster dose not exsists in table, bond energy is utilized
#
if __name__ == "__main__":
    outdir = 'AlSi60'
    etbfile = 'Al-Si-V.etb'
    csvfile = 'Al-Si-V.csv'
    isolates = { 0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    ebonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0 }
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    work = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    cid = work.grid2index(0, 1, 1, 1)
    ctypes = qetool.countQEoutClusterTypes(outdir)
    results = []
    # Al cluster
    outfile = 'AlSi60/qAAAAAAAAAAAAA.out'
    data = qetool.readQEout(outfile)
    tote = qetool.BindingEnergy(data, isolates)
    s = os.path.splitext(os.path.basename(outfile))[0]
    types = MPKMC.string2types(s)
    kmc.add_cluster(types, tote / data['NAtom'], 1)
    # Select Al-Si cluster
    alsi = [t for t in ctypes if not 0 in t[1] and 14 in t[1]]
    alsi.sort(key=lambda n: n[1][14])
    ret = qetool.fillUnkownClusters2(kmc, work, alsi, isolates, ebonds, cid)
    results.extend(ret)
    # Select Al-V cluster
    alv = [t for t in ctypes if not 14 in t[1] and 0 in t[1]]
    alv.sort(key=lambda n: n[1][0])
    ret = qetool.fillUnkownClusters2(kmc, work, alv, isolates, ebonds, cid)
    results.extend(ret)
    # Select Al-Si-V cluster
    alsiv = [t for t in ctypes if 13 in t[1] and 14 in t[1] and 0 in t[1]]
    alsiv.sort(key=lambda n: n[1][14])
    alsiv.sort(key=lambda n: n[1][0])
    ret = qetool.fillUnkownClusters2(kmc, work, alsiv, isolates, ebonds, cid)
    results.extend(ret)
    # Compare
    results2 = []
    kmc.table_add = False
    for outfile, clue, bcount, add, tote in results:
        data = qetool.readQEout(outfile)
        qetool.changeType(kmc, data)
        kmc.grid_energy(dummyQE)
        diff = abs(kmc.tote-tote)
        results2.append((outfile, clue, bcount, add, tote, kmc.tote, diff))
        if diff > 0.1:
            print(outfile, bcount, diff)
    # Write Results
    kmc.sort_table()
    kmc.write_table(etbfile)
    df = pd.DataFrame(results2, columns=['File', 'Energy', 'BCount', 'Add', 'QE', 'MC', 'Error'])
    df.to_csv(csvfile)
    print('Error Max:', df['Error'].max())



