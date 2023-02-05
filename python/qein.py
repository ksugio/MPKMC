import MPKMC
import qetool
import os

#
# genarate .in file
#
if __name__ == "__main__":
    qedir = 'qeAlV'
    headfile = 'qehead.in'
    kmc = qetool.FCCCell(13, 4.04360, 2, 2, 2, 32, 1024, 1, 1)
    cid = kmc.grid2index(0, 1, 1, 1)
    # Al-Si
    #ltypes = [ {14: i} for i in range(1, 14) ]
    ltypes = [ {0: i} for i in range(1, 14) ]
    print(ltypes)
    for types in ltypes:
        qetool.buildClusters(kmc, types, cid)
    # Write .in file
    for i in range(kmc.ntable):
        clu = kmc.table_item(i)[0]
        infile = '%s/%05d%s.in' % (qedir, i, MPKMC.types2string(clu))
        qetool.writeQEin(clu, kmc.cluster, headfile, infile, fill_type=13)

