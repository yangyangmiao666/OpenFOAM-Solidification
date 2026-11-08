# Porting OpenFOAM-Solidification to OpenFOAM 14

Source: 2018 Foundation `solidificationFoam` + `SMACS_I` (Bennon–Incropera binary alloy).
Target: OpenFOAM 14 (Foundation) on Multipass VM `openfoam`.

Physics is preserved. This does **not** use `fvModels/solidificationMelting`.

## Major API changes addressed

| Area | 2018 | OpenFOAM 14 port |
|------|------|------------------|
| `twoPhaseMixture` | Base class with `(mesh, dict)` ctor | **Not inherited** — OF14 mixture owns `phaseProperties` conversion and pure-virtual `rho()/correct()`. `solidificationSystem` is a standalone `IOdictionary` that still reads `constant/transportProperties` and `alpha.solid`. |
| Umbrella include | `fvCFD.H` | Explicit includes (`argList`, `fvm*`, `fvc*`, `pimpleControl`, …) like modern legacy solvers |
| Time directory | `runTime.timeName()` | `runTime.name()` / `userTimeName()` |
| Flux required | `mesh.setFluxRequired` | `mesh.schemes().setFluxRequired` |
| Sources/constraints | `fvOptions` | `fvModels` + `fvConstraints` |
| Gravity / `gh` | `readhRef.H` + `gh.H` | Inlined (`hRef`, `gh`, `ghf`) matching OF14 `buoyancy` |
| MRF | `correctBoundaryVelocity`, `zeroFilter` | Removed; keep `DDt` + `makeRelative` |
| BC mapping | `fvPatchFieldMapper`, `autoMap`/`rmap` | `fieldMapper`, `map`/`reset`; `DimensionedField<..., fvMesh>` |
| Field write | `field.writeEntry(name, os)` | `writeEntry(os, name, field)` |
| Residuals func | `caseDicts/postProcessing/...` | `caseDicts/functions/numerical/residuals.cfg` |
| PIMPLE loop | `while (runTime.run())` + `pimple.loop()` | `while (pimple.run(runTime))` + `pimple.loop()` |
| Make libs | compressible/turbulence transport | `finiteVolume`, `meshTools`, `fvModels`, `fvConstraints`, `libsolidificationSystem` |

## Build

```bash
multipass exec openfoam -- bash -lc '
  . /opt/openfoam14/etc/bashrc
  cd /home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/applications/solvers/solidificationFoam
  ./Allwclean
  ./Allwmake -j
'
```

Binary: `$FOAM_USER_APPBIN/solidificationFoam`  
Library: `$FOAM_USER_LIBBIN/libsolidificationSystem.so`

## Run SMACS_I (serial bring-up)

`system/controlDict` ships with `endTime 0.05` (10 steps at `deltaT 0.005`) for verification.
For the full 600 s case, set `endTime 600;` and `writeInterval 1;`.

```bash
multipass exec openfoam -- bash -lc '
  . /opt/openfoam14/etc/bashrc
  cd /home/ubuntu/OpenFOAM/ubuntu-14/run/OpenFOAM-Solidification/tutorials/multiphase/solidificationFoam/SMACS_I
  ./Allclean
  ./Allrun
'
```

Parallel (4 ranks, see `decomposeParDict`):

```bash
./Allrun-parallel
```

## Files changed (solver)

- `applications/solvers/solidificationFoam/solidificationSystem/*`
- `applications/solvers/solidificationFoam/BCs/robinTemperature*`
- `applications/solvers/solidificationFoam/solidificationFoam.C`
- `createFields.H`, `createAuxilaryFields.H`, `createFieldRefs.H`, `readThermoProperties.H`, `thermoLoop.H`
- `thermo/*`, `pU/*`, `Make/options`, `Make/files`

## Files changed (tutorial)

- `SMACS_I/system/controlDict` (OF14 FoamFile, short `endTime`, residuals)
- `SMACS_I/system/residuals` (include path)
- `SMACS_I/Allrun` (serial), `Allrun-parallel` (new)
- FoamFile version stamps on case dicts / fields

## Notes

- Keep host Clash/Verge proxy on `7897` if the VM needs network during builds.
- Production physics: restore `endTime 600` in `controlDict`.


## Bring-up verification (2026-11-08 Asia/Shanghai)

Serial SMACS_I with `endTime 0.05`, `deltaT 0.005` (10 steps):

- Binary: `/home/ubuntu/OpenFOAM/ubuntu-14/platforms/linuxArm64GccDPInt32Opt/bin/solidificationFoam`
- Library: `/home/ubuntu/OpenFOAM/ubuntu-14/platforms/linuxArm64GccDPInt32Opt/lib/libsolidificationSystem.so`
- Completed with `End` in ~0.86 s CPU; wrote `0.05/`
- Thermo loop typically converges in 1–2 iters (`THERMO residual` → `<1e-4`)
- Linear solvers reach ~1e-8/1e-9 (T, C, U, p_rgh)
- Sample residuals (`postProcessing/residuals/0/residuals.dat`):
  - t=0.005: T≈1.0, C≈2e-4, Ux≈1.0, p_rgh≈1.0
  - t=0.05:  T≈4.8e-2, C≈1.1e-1, Ux≈5.6e-1, p_rgh≈2.8e-2
- **Note:** with fixed `deltaT=0.005`, max Courant grows (≈3 → ≈79 by t=0.05). For longer runs, lower `deltaT` or enable `adjustTimeStep` / `maxCo` before restoring `endTime 600`.

Mac sync location: `/Users/yangyangmiao/myy/github_projects/OpenFOAM-Solidification` (`.git` preserved).
