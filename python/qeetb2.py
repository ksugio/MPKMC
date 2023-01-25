import qetool
import pandas as pd

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
    ctypes = qetool.countQEoutClusterTypes(outdir)
    results = []
    # Select Al only
    al = [t for t in ctypes if not 0 in t[1] and not 14 in t[1]]
    ret = qetool.fillUnkownClustersByOrder(kmc, al, isolates, ebonds)
    results.extend(ret)
    # Select Al-Si cluster
    alsi = [t for t in ctypes if not 0 in t[1] and 14 in t[1]]
    alsi.sort(key=lambda n: n[1][14])
    ret = qetool.fillUnkownClustersByOrder(kmc, alsi, isolates, ebonds)
    results.extend(ret)
    # Select Al-V cluster
    alv = [t for t in ctypes if not 14 in t[1] and 0 in t[1]]
    alv.sort(key=lambda n: n[1][0])
    ret = qetool.fillUnkownClustersByOrder(kmc, alv, isolates, ebonds)
    results.extend(ret)
    # Select Al-Si-V cluster
    alsiv = [t for t in ctypes if 13 in t[1] and 14 in t[1] and 0 in t[1]]
    alsiv.sort(key=lambda n: n[1][14])
    alsiv.sort(key=lambda n: n[1][0])
    ret = qetool.fillUnkownClustersByOrder(kmc, alsiv, isolates, ebonds)
    results.extend(ret)
    # Write Results
    kmc.reset_table()
    kmc.write_table(etbfile)
    df = pd.DataFrame(results, columns=['File', 'Count', 'QE', 'MC', 'Error'])
    df.to_csv(csvfile)
    print('Error Max:', df['Error'].max())



