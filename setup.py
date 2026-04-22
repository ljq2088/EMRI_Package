import os
from pathlib import Path

from setuptools import Extension, setup, find_packages

import pybind11


CONDA_ROOT = Path("/home/ljq/miniconda3")
GSL_ROOT = Path(os.environ.get("EMRI_GSL_ROOT", CONDA_ROOT / "pkgs" / "gsl-2.8-hbf7d49c_1"))
EIGEN_ROOT = Path(
    os.environ.get(
        "EMRI_EIGEN_ROOT",
        "/home/ljq/code/Teukolsky_based/GremLinEqRe/build/_deps/eigen3-src",
    )
)


ext_modules = [
    Extension(
        "emri_package._orbit",
        sources=[
            "cpp/emri_package/bindings/module.cpp",
            "cpp/emri_package/emri_params.cpp",
            "cpp/emri_package/orbit/fundamental_frequencies.cpp",
            "cpp/emri_package/orbit/nk_orbit.cpp",
        ],
        include_dirs=[
            "cpp/emri_package",
            "cpp",
            str(GSL_ROOT / "include"),
            pybind11.get_include(),
            pybind11.get_include(user=True),
        ],
        library_dirs=[str(GSL_ROOT / "lib")],
        libraries=["gsl", "gslcblas", "m"],
        runtime_library_dirs=[str(GSL_ROOT / "lib")],
        extra_compile_args=["-O3", "-std=c++17"],
        language="c++",
    ),
    Extension(
        "emri_package._teukolsky",
        sources=[
            "cpp/emri_package/bindings/teukolsky_module.cpp",
            "cpp/emri_package/teukolsky/KerrGeo.cpp",
            "cpp/emri_package/teukolsky/SWSH_LRR.cpp",
            "cpp/emri_package/teukolsky/TeukolskySource.cpp",
        ],
        include_dirs=[
            "cpp/emri_package/include",
            "cpp/emri_package/teukolsky",
            str(EIGEN_ROOT),
            pybind11.get_include(),
            pybind11.get_include(user=True),
        ],
        libraries=["m"],
        extra_compile_args=["-O3", "-std=c++17"],
        language="c++",
    ),
]


setup(
    name="emri_package",
    version="0.1.0",
    description="Minimal EMRI integration package with extracted orbit kernel",
    package_dir={"": "src"},
    packages=find_packages(where="src"),
    ext_modules=ext_modules,
    zip_safe=False,
)
