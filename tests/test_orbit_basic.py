import numpy as np

from emri_package import KerrOrbit, bl_to_cartesian


def main():
    orbit = KerrOrbit(M=1.0e6, a=0.9, p=10.0, e=0.3, iota=np.pi / 4.0, mu=0.0)
    constants = orbit.conserved_quantities()
    assert constants.E > 0.0, "mapping failed"

    traj = orbit.evolve(duration=200.0, dt=2.0)
    assert len(traj.t) > 10, "trajectory too short"
    assert np.all(np.isfinite(traj.r))
    assert np.all(np.isfinite(traj.theta))

    x, y, z = bl_to_cartesian(traj.r, traj.theta, traj.phi)
    assert x.shape == traj.t.shape
    assert np.isfinite(x).all() and np.isfinite(y).all() and np.isfinite(z).all()

    print("basic orbit test passed")
    print(f"steps={len(traj.t)}")
    print(f"r_range=({traj.r.min():.6f}, {traj.r.max():.6f})")
    print(f"phi_final={traj.phi[-1]:.6f}")


if __name__ == "__main__":
    main()
