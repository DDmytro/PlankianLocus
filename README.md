# PlankianLocus
A simple OpenCL training application which demonstrates the CIE chromaticity diagram and the Planck locus curve drawing. Cross-platform compilation is supported.

# Requirements
OpenCL SDK that support at least OpenCL 1.2

# Features
- Crossplatform.
- OpenCL implementation.
- Plankian locus 7-order polinominal approximation. 
- OpenCL bresenham line algorithm.

# Build

## Windovs
- Create Visual Studio Solution.
`./premake/win/premake5 vs2015` or `vs2013, vs2012,...`
- Build solution.

## Linux
- `./premake/linux64/premake5 gmake`
- `make config=release` or `make config=debug`

## OSX
Not tested!
- Create Xcode Solution.
`./premake/linux64/premake5 xcode4` 
- Or
- `./Tools/premake/osx/premake5 gmake` 
- `make config=release or make config=debug` 

# Run
- `./PlankianLocus` without arguments. The output planckianLocus.ppm file will be stored into application folder.
