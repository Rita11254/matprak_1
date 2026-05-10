import os
import os.path
import openseespy.opensees as ops

ops.wipe()
ops.model('basic', '-ndm', 2, '-ndf', 2)

L = 0.146
H = L * 2
h = L / 40

numx = 3.0
numy = 3.0
rho = 1000.0
mu = 0.0001
b1 = 0.0
b2 = -9.81
thk = 0.012
kappa = -1.0
dtmax = 1e-3
dtmin = 1e-3
totaltime = 1.0
filename = 'dambreak-bg'

ops.recorder('BgPVD', filename, 'disp', 'vel', 'pressure', '-dT', 1e-3)
if not os.path.exists(filename):
    os.makedirs(filename)

ndf = 2
nx = round(L / h * numx)
ny = round(H / h * numy)

eleArgs = ['PFEMElementBubble', rho, mu, b1, b2, thk, kappa]
partArgs = ['quad', 0.0, 0.0, L, 0.0, L, H, 0.0, H, nx, ny]
parttag = 1
ops.mesh('part', parttag, *partArgs, *eleArgs, '-vel', 0.0, 0.0)

ops.node(1, 0.0, H)
ops.node(2, 0.0, 0.0)
ops.node(3, 4 * L, 0.0)
ops.node(4, 4 * L, H)

walltag = 2
wallid = 1
ops.mesh('line', walltag, 4, 1, 2, 3, 4, wallid, ndf, h)

wallnodes = ops.getNodeTags('-mesh', walltag)

for nd in wallnodes:
    ops.fix(nd, 1, 1)

lower = [-h, -h]
upper = [4 * L + L, H + L]

ops.mesh('bg', h, *lower, *upper,
         '-structure', wallid, len(wallnodes), *wallnodes)

ops.constraints('Plain')
ops.numberer('Plain')
ops.test('PFEM', 1e-5, 1e-5, 1e-5, 1e-5, 1e-5, 1e-5, 10, 3, 1, 2)

ops.algorithm('Newton')
ops.integrator('PFEM', 0.5, 0.25)

ops.system('PFEM')
ops.analysis('PFEM', dtmax, dtmin, b2)

while ops.getTime() < totaltime:
    if ops.analyze() < 0:
        break
    ops.remesh()
