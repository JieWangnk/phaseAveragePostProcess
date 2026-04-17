# phaseAveragePostProcess

Classical offline phase averaging utility for OpenFOAM 12.

## What it does

Given a completed periodic simulation with snapshots written at regular intervals over multiple cycles, this utility computes the arithmetic ensemble mean of fields at each phase point across cycles:

```
UMean(phase) = (1/N) * sum_{n=1}^{N} U(t_phase + n*T)
```

This is the standard pointwise phase-averaging procedure used in cardiovascular, turbomachinery, and other periodic-flow applications. It reads existing time directories, accumulates the sum, divides by N, and writes the result. No equations are solved.

## Background

In OpenFOAM 4, offline phase averaging was done by running a custom solver (e.g. `pimpleFoam`) with the `-postProcess` flag and a `fieldAverage` function object. In OpenFOAM 12, the `-postProcess` flag was removed from most solvers and `foamPostProcess` does not support `fieldAverage` (which needs a time loop to accumulate). This utility fills that gap.

## Repository contents

```
src/
  phaseAveragePostProcess/    The utility (reads fields, sums, divides by N)
  WKBCCompat/                 Compatibility library for reading old OF4 cases
                              that use the "WKBC" pressure boundary condition
scripts/
  phaseAverageOF12.sh         Wrapper script for the full phase-averaging
                              workflow (directory creation, symlinking, running)
```

## Build

Requires OpenFOAM 12 (Foundation, openfoam.org).

```bash
# The utility
cd src/phaseAveragePostProcess
wmake

# Optional: WKBC compatibility library (only for old OF4 case files)
cd ../WKBCCompat
wmake libso
```

## Usage

### Quick (single phase directory)

If you have already set up a phase directory with symlinked time directories from different cycles:

```bash
cd /path/to/phase/0.32
phaseAveragePostProcess -time '0.32,1.13,1.94,2.75,3.56'
```

This reads U and p from each listed time directory, computes the arithmetic mean, and writes `UMean` and `pMean` to the last time directory.

The `-time` flag with an explicit comma-separated list is required to prevent OpenFOAM's time selector from scanning the parent case through symlinks.

### Full workflow (all phases)

```bash
cd /path/to/parent/case
bash /path/to/scripts/phaseAverageOF12.sh
```

Edit the parameters at the top of `phaseAverageOF12.sh` first:

```bash
endT=0.809          # last phase point (T - dt_write)
ncycles_start=1     # first cycle to include (skip 0 for transient)
ncycles=25          # total cycles
T=0.81              # period (seconds)
dt_write=0.01       # snapshot interval
```

The script creates one directory per phase point, symlinks the corresponding time directories from each cycle, and runs `phaseAveragePostProcess` for each phase.

### Custom boundary conditions

The utility works with any boundary condition type. If the case uses a custom BC provided by a runtime library, load it with `-libs`:

| Case type | Pressure BC | Command |
|---|---|---|
| Simple (zeroGradient, fixedValue) | Built-in | `phaseAveragePostProcess -time '...'` |
| OF12 with modularWKPressure | `modularWKPressure` | `phaseAveragePostProcess -time '...' -libs '("libmodularWKPressure.so")'` |
| Old OF4 cases with WKBC | `WKBC` | `phaseAveragePostProcess -time '...' -libs '("libWKBCCompat.so")'` |

The `WKBCCompat` library in `src/WKBCCompat/` is only needed for backward compatibility with old OpenFOAM 4 case files that use `type WKBC;` in their pressure boundary. For any new simulation run in OpenFOAM 12, it is not needed.

## Output

Each phase directory contains:
- `UMean` — ensemble-mean velocity at that phase
- `pMean` — ensemble-mean pressure
- Any other volScalarField or volVectorField present in the time directories

## Relation to BBPA

This utility computes **pointwise** phase averages — each phase point is a single time instant with no temporal bin width. It is the offline equivalent of the IPA (Instantaneous Phase Averaging) function object.

[BBPA](https://github.com/JieWangnk/Bin-Phase-Average) (Bin-Based Phase Averaging) is a different approach: it divides the cycle into bins of finite width, averages all solver steps within each bin, and accumulates in memory during the simulation. BBPA trades a bounded quantization error for the ability to run entirely in-situ without writing snapshots.

For long-duration simulations where snapshot storage is the bottleneck, use BBPA or IPA in-situ. For post-processing existing snapshot archives, use this utility.

## License

MIT
