import numpy as np

from emri_package import KerrOrbit


def metric_norm(a, r, theta, ut, ur, utheta, uphi):
    sigma = r * r + a * a * np.cos(theta) ** 2
    delta = r * r - 2.0 * r + a * a
    sin2 = np.sin(theta) ** 2
    g_tt = -(1.0 - 2.0 * r / sigma)
    g_tphi = -2.0 * a * r * sin2 / sigma
    g_rr = sigma / delta
    g_thth = sigma
    g_phiphi = (((r * r + a * a) ** 2 - delta * a * a * sin2) * sin2) / sigma
    return g_tt * ut * ut + 2.0 * g_tphi * ut * uphi + g_rr * ur * ur + g_thth * utheta * utheta + g_phiphi * uphi * uphi


def main():
    a = 0.9
    orbit = KerrOrbit(M=1.0e6, a=a, p=10.0, e=0.3, iota=np.pi / 4.0, mu=0.0)
    traj, kin = orbit.evolve_with_kinematics(duration=200.0, dt=2.0)

    assert len(traj.t) == len(kin.t) > 10
    assert np.all(np.isfinite(kin.E))
    assert np.all(np.isfinite(kin.Lz))
    assert np.all(np.isfinite(kin.Q))
    assert np.all(np.isfinite(kin.ut))
    assert np.all(np.isfinite(kin.uphi))

    i = len(kin.t) // 2
    norm = metric_norm(a, kin.r[i], kin.theta[i], kin.ut[i], kin.ur[i], kin.utheta[i], kin.uphi[i])
    assert abs(norm + 1.0) < 5e-4, f"4-velocity normalization failed: {norm}"

    print("kinematics test passed")
    print(f"E={kin.E[i]:.8f}, Lz={kin.Lz[i]:.8f}, Q={kin.Q[i]:.8f}")
    print(f"u_norm={norm:.8e}")


if __name__ == "__main__":
    main()
