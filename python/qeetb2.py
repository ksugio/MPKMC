import qetool
import pandas as pd

def dummyQE(kmc, types):
    ebonds = {(13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0}
    return qetool.bondClusterEnergy(types, ebonds)

def Compare(kmc, results):
    results2 = []
    kmc.table_add = False
    for outfile, clue, unknown, add, tote in results:
        data = qetool.readQEout(outfile)
        qetool.changeType(kmc, data)
        kmc.grid_energy(dummyQE)
        diff = abs(kmc.tote-tote)
        results2.append((outfile, clue, unknown, add, tote, kmc.tote, diff))
    kmc.sort_table()
    return results2

def WriteResults(results, resfile):
    df = pd.DataFrame(results, columns=['File', 'Energy', 'Unknown', 'Add', 'QE', 'MC', 'Error'])
    df.to_csv(resfile)
    print('Number of Results:', len(df))
    print('Maximum Error:', df['Error'].max())
    print('Mean Error:', df['Error'].mean())
    print('STD of Error:', df['Error'].std())

#
# genarate .etb file from QE .out files with cluster energy
# if the cluster dose not exsists in table, bond energy is utilized
#
if __name__ == "__main__":
    outdir = 'AlSi60'
    etbfile = 'Al-Si-V.etb'
    resfile = 'Al-Si-V-res.csv'
    isolates = { 0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    ebonds = { (13, 13): -0.04506907817708337, (13, 14): -0.05215988817708158,
              (14, 14): -0.058415198177092176, (0, 13): -0.01913845484375054,
              (0, 14): -0.03314451484373948, (0, 0): 0.0 }
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    cid = kmc.grid2index(0, 1, 1, 1)
    ctypes = qetool.countQEoutClusterTypes(outdir)
    results = []
    # Select Al only
    al = [t for t in ctypes if not 0 in t[1] and not 14 in t[1]]
    #al1si = [t for t in ctypes if not 0 in t[1] and 14 in t[1] and t[1][14] == 1]
    #al.extend(al1si)
    ret = qetool.fillUnkownClusters(kmc, al, isolates, ebonds, cid)
    results.extend(ret)
    # Select Al-Si cluster
    alsi = [t for t in ctypes if not 0 in t[1] and 14 in t[1]]
    alsi.sort(key=lambda n: n[1][14])
    ret = qetool.fillUnkownClusters(kmc, alsi, isolates, ebonds, cid)
    results.extend(ret)
    # Select Al-V cluster
    alv = [t for t in ctypes if not 14 in t[1] and 0 in t[1]]
    alv.sort(key=lambda n: n[1][0])
    ret = qetool.fillUnkownClusters(kmc, alv, isolates, ebonds, cid)
    results.extend(ret)
    # Select Al-Si-V cluster
    alsiv = [t for t in ctypes if 13 in t[1] and 14 in t[1] and 0 in t[1]]
    alsiv.sort(key=lambda n: n[1][14])
    alsiv.sort(key=lambda n: n[1][0])
    ret = qetool.fillUnkownClusters(kmc, alsiv, isolates, ebonds, cid)
    results.extend(ret)
    results = Compare(kmc, results)
    WriteResults(results, resfile)
    kmc.write_table(etbfile)
