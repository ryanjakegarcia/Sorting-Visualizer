# Sorting Visualizer

An interactive sorting visualizer built with Raylib. It animates a range of sorting algorithms, shows benchmark results, and supports loading benchmark profiles that tune each sort for a more comparable showcase.

## Build

Prerequisites:
- `gcc`
- `pkg-config`
- `raylib`

Build the app:

```bash
make
```

Run it:

```bash
make run
```

Clean the build output:

```bash
make clean
```

## Using the App

When the app starts, it shows a splash screen with a live preview of the sorting algorithms. Click the splash art box or press the start action shown on-screen to enter the main visualizer.

In the main visualizer you can:
- watch the current sort animate on the array
- open the benchmark overlay
- view per-sort statistics and timing
- switch between the available algorithms through the on-screen controls

The app is designed to show both the algorithm motion and the benchmark behavior, so you can compare how different sorts behave on the same dataset.

## Benchmark Profiles

Benchmark profiles let you load a saved benchmark setup that controls the conditions for each sort. The profile format is CSV and uses a global header section followed by one row per sort.

The included example profile is:

```text
benchmarks/benchmark_profile.csv
```

### What a profile can control

Global settings in the header section:
- random seed
- distribution mode
- runs per sort
- warmup on or off
- default UI mode

Per-sort settings in each row:
- target display time
- array size `N`
- speed factor
- enabled or disabled
- per-sort UI mode

### How the tuning works

The benchmark loader uses a heuristic based on sort class:
- quadratic sorts
- `n log n` sorts
- linear sorts
- a special case for bogosort

Each sort row stores a target display time, and the app chooses a suitable `N` and speed factor from that target so each sort is shown for roughly the same amount of time.

### Loading a profile

Open the benchmark overlay and load the profile from there. The app also supports loading the profile with the on-screen load action key shown in the overlay.

If the profile file is missing, the app falls back to built-in benchmark defaults.

### Example profile format

```csv
# benchmark_profile_v1
# global, key, value
global,seed,1337
global,distribution,random
global,runs_per_sort,3
global,warmup,0
global,default_ui_mode,minimal

# sort,name,key,value pairs
sort,Quick,enabled,1,target_seconds,4.0,n,256,speed,1.0,ui_mode,minimal
sort,Bogosort,enabled,1,target_seconds,4.0,n,8,speed,0.7,ui_mode,minimal
```

## Files of Interest

- [src/visualizer.c](src/visualizer.c) - main app loop, benchmark overlay, profile loading, and splash behavior
- [src/sorts.c](src/sorts.c) - sorting implementations
- [src/ui.c](src/ui.c) - rendering logic
- [benchmarks/benchmark_profile.csv](benchmarks/benchmark_profile.csv) - default benchmark profile
