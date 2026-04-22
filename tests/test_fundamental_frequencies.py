import numpy as np

from emri_package import KerrOrbit, fundamental_frequencies_from_constants


def estimate_radial_frequency(t: np.ndarray, r: np.ndarray) -> float:
    minima_times = []
    for i in range(1, len(r) - 1):
        if r[i] <= r[i - 1] and r[i] < r[i + 1]:
            minima_times.append(t[i])
    if len(minima_times) < 2:
        raise RuntimeError("Not enough radial turning points to estimate Omega_r")
    periods = np.diff(minima_times)
    return 2.0 * np.pi / np.mean(periods)


def main():
    orbit = KerrOrbit(M=1.0e6, a=0.9, p=10.0, e=0.3, iota=1.0, mu=0.0)
    consts = orbit.conserved_quantities()
    freqs = fundamental_frequencies_from_constants(orbit.a, consts.E, consts.Lz, consts.Q)

    traj = orbit.evolve(duration=2000.0, dt=0.5)
    omega_phi_est = (traj.phi[-1] - traj.phi[0]) / (traj.t[-1] - traj.t[0])
    omega_r_est = estimate_radial_frequency(traj.t, traj.r)

    assert np.isfinite(freqs.Omega_r)
    assert np.isfinite(freqs.Omega_theta)
    assert np.isfinite(freqs.Omega_phi)
    assert freqs.Omega_r > 0.0
    assert freqs.Omega_theta > 0.0
    assert freqs.Gamma > 0.0
    assert abs(freqs.Omega_phi - omega_phi_est) / omega_phi_est < 0.08
    assert abs(freqs.Omega_r - omega_r_est) / omega_r_est < 0.12

    print("fundamental frequencies test passed")
    print(f"Omega_r={freqs.Omega_r:.8f}, est={omega_r_est:.8f}")
    print(f"Omega_theta={freqs.Omega_theta:.8f}")
    print(f"Omega_phi={freqs.Omega_phi:.8f}, est={omega_phi_est:.8f}")


if __name__ == "__main__":
    main()
