import qetool

# find out file
def searchOut():
    outdir = 'AlSi60'
    for path, types in qetool.listQEoutTypes(outdir):
        if 0 in types and 14 in types:
            if types[0] == 1 and types[14] == 1:
                print(path, types)

if __name__ == "__main__":
    # searchOut()
    isolates = { 0: 0.0, 13: -4.76961123, 14: -11.02436232, }
    outfiles = [ ('AlSi60/qAAAAAAAAAAAAA.out', (13, 13)),
                 ('AlSi60/qAAAAAASAAAAAA.out', (13, 14)),
                 ('AlSi60/qSASAAAAAAAAAA.out', (14, 14)),
                 ('AlSi60/qAA0AAAAAAAAAA.out', (0, 13)),
                 ('AlSi60/qAS0AAAAAAAAAA.out', (0, 14)) ]
    ebonds = {}
    for outfile, combi in outfiles:
        bonds, ebonds = qetool.BondEnergy(outfile, combi, isolates, ebonds)
        print(bonds)
    print(ebonds)
