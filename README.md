# EMRI_Package

Minimal integration package for building an EMRI pipeline around:

- Kerr/NK geodesic and inspiral orbit generation extracted from `Kludge_EMRI`
- future angular solver integration using `Teukolsky_based/GremLinEqRe/include/SWSH_LRR.h`
- future radial homogeneous solution integration using PINN inference from `PINN/SolvingTeukolsky`

Current status:

- `orbit` is extracted and buildable
- Python wrapper API is stable enough to be reused by later Teukolsky modules
- `angular` and `radial` expose adapter interfaces and documented integration points

## Build

Requirements:

- Python 3.10+
- `setuptools`
- `pybind11`
- GSL development libraries (`gsl`, `gslcblas`)
- a C++17 compiler

Build the extension in place:

```bash
cd /home/ljq/code/EMRI_Package
python setup.py build_ext --inplace
```

Run the basic orbit test:

```bash
PYTHONPATH=src python tests/test_orbit_basic.py
```

## Layout

- `cpp/emri_package/orbit`: extracted C++ orbit kernel
- `cpp/emri_package/bindings`: pybind11 bindings
- `src/emri_package/orbit.py`: high-level Python orbit wrapper
- `src/emri_package/angular.py`: angular solver interface for future `SWSH_LRR` integration
- `src/emri_package/radial.py`: radial solver interface for future PINN integration

## Notes

The current 3D orbit plotting should still be done on the Python side by converting
`(r, theta, phi)` into pseudo-flat Cartesian coordinates:

```python
x = r * np.sin(theta) * np.cos(phi)
y = r * np.sin(theta) * np.sin(phi)
z = r * np.cos(theta)
```
