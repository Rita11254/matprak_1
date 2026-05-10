import os
import openseespy.opensees as ops

ops.wipe()
ops.model('basic', '-ndm', 2, '-ndf', 3)

L = 0.146
H = L * 2
H2 = 0.3
b = 0.03
h = L / 40
Hb = 20.0 * b / 6.0 * 0.75

numx = 3.0
numy = 3.0
rho = 1000.0
mu = 0.0001
b1 = 0.0
b2 = -9.81
thk = 0.012
thk_me = 0.006
kappa = -1.0
rhos = 2400.0
A = thk_me * thk_me
E = 30e9
Iz = thk_me * thk_me * thk_me * thk_me / 12.0
bmass = A * Hb * rhos

E0 = 30e6
Fy = 30e6
hardening = 0.02

nonlinear = True

dtmax = 1e-3
dtmin = 1e-3
totaltime = 1.0

if nonlinear:
    filename = 'obstaclenonlinear-bg'
else:
    filename = 'obstacle-bg'

ops.recorder('BgPVD', filename, 'disp', 'vel', 'pressure', '-dT', 1e-3)
if not os.path.exists(filename):
    os.makedirs(filename)

ndf = 3
nx = round(L / h * numx)
ny = round(H / h * numy)

eleArgs = ['PFEMElementBubble', rho, mu, b1, b2, thk, kappa]
partArgs = ['quad', 0.0, 0.0, L, 0.0, L, H, 0.0, H, nx, ny]
parttag = 1
ops.mesh('part', parttag, *partArgs, *eleArgs, '-vel', 0.0, 0.0)

ops.node(1, 2 * L, 0.0)
ops.node(2, 2 * L, Hb)
ops.node(3, 0.0, H)
ops.node(4, 0.0, 0.0)
ops.node(5, 4 * L, 0.0)
ops.node(6, 4 * L, H)

sid = 1
walltag = 4
ops.mesh('line', walltag, 5, 3, 4, 1, 5, 6, sid, ndf, h)

wallNodes = ops.getNodeTags('-mesh', walltag)
for nd in wallNodes:
    ops.fix(nd, 1, 1, 1)

transfTag = 1
ops.geomTransf('Corotational', transfTag)

matTag = 1
mom1p = Fy
rot1p = E0
mom2p = Fy * 0.8
rot2p = E0 * 2
mom3p = 0.0
rot3p = E0 * 5
mom1n = -mom1p
rot1n = -rot1p
mom2n = -mom2p
rot2n = -rot2p
mom3n = 0.0
rot3n = -E0 * 5
pinchX = 0.5
pinchY = 0.5
damfc1 = 0.0
damfc2 = 0.0
beta = 0.0

secTag = 1
if nonlinear:
    matTag = 1
    ops.uniaxialMaterial('Hysteretic', matTag, 
                     mom1p, rot1p, mom2p, rot2p, mom3p, rot3p,
                     mom1n, rot1n, mom2n, rot2n, mom3n, rot3n,
                     pinchX, pinchY, damfc1, damfc2, beta)
    numfiber = 5
    ops.section('Fiber', secTag)
    ops.patch('rect', matTag, numfiber, numfiber, 0.0, 0.0, thk_me, thk_me)
else:
    ops.section('Elastic', secTag, E, A, Iz)

inteTag = 1
numpts = 2
ops.beamIntegration('Legendre', inteTag, secTag, numpts)

coltag = 3
eleArgs = ['dispBeamColumn', transfTag, inteTag]
ops.mesh('line', coltag, 2, 1, 2, sid, ndf, h, *eleArgs)

sNodes = ops.getNodeTags('-mesh', coltag)
bmass = bmass / len(sNodes)
for nd in sNodes:
    ops.mass(int(nd), bmass, bmass, 0.0)

lower = [-h, -h]
upper = [5 * L, 3 * L]

ops.mesh('bg', h, *lower, *upper,
     '-structure', sid, len(sNodes), *sNodes,
     '-structure', sid, len(wallNodes), *wallNodes)

ops.constraints('Plain')
ops.numberer('Plain')
ops.test('PFEM', 1e-5, 1e-5, 1e-5, 1e-5, 1e-5, 1e-5, 100, 3, 1, 2)
ops.algorithm('Newton')
ops.integrator('PFEM', 0.5, 0.25)

ops.system('PFEM')
ops.analysis('PFEM', dtmax, dtmin, b2)

while ops.getTime() < totaltime:
    if ops.analyze() < 0:
        break
    ops.remesh()
