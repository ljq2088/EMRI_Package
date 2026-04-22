from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

from emri_package import KerrOrbit, bl_to_cartesian


def plot_orbit(
    output_path: str | Path = "orbit_3d_plot.png",
    M: float = 1.0e6,
    a: float = 0.9,
    p: float = 10.0,
    e: float = 0.7,
    iota: float = np.pi / 3.0,
    mu: float = 0.0,
    duration: float = 400.0,
    dt: float = 1.0,
):
    orbit = KerrOrbit(M=M, a=a, p=p, e=e, iota=iota, mu=mu)
    traj = orbit.evolve(duration=duration, dt=dt)
    x, y, z = bl_to_cartesian(traj.r, traj.theta, traj.phi)

    fig = plt.figure(figsize=(10, 8))
    ax = fig.add_subplot(111, projection="3d")
    ax.plot(x, y, z, lw=1.5, color="blue", label="Test Particle Trajectory")
    ax.scatter(x[0], y[0], z[0], color="green", s=50, label="Start")
    ax.scatter(x[-1], y[-1], z[-1], color="red", s=50, label="End")

    r_plus = 1.0 + np.sqrt(1.0 - a**2)
    u, v = np.mgrid[0 : 2 * np.pi : 20j, 0 : np.pi : 10j]
    bh_x = r_plus * np.cos(u) * np.sin(v)
    bh_y = r_plus * np.sin(u) * np.sin(v)
    bh_z = r_plus * np.cos(v)
    ax.plot_surface(bh_x, bh_y, bh_z, color="black", alpha=0.3)

    ax.set_xlabel("x (M)")
    ax.set_ylabel("y (M)")
    ax.set_zlabel("z (M)")
    ax.set_title("NK/Kerr Geodesic Trajectory")

    max_range = np.array([x.max() - x.min(), y.max() - y.min(), z.max() - z.min()]).max() / 2.0
    mid_x = (x.max() + x.min()) * 0.5
    mid_y = (y.max() + y.min()) * 0.5
    mid_z = (z.max() + z.min()) * 0.5
    ax.set_xlim(mid_x - max_range, mid_x + max_range)
    ax.set_ylim(mid_y - max_range, mid_y + max_range)
    ax.set_zlim(mid_z - max_range, mid_z + max_range)

    plt.legend()
    plt.tight_layout()
    output_path = Path(output_path)
    plt.savefig(output_path, dpi=150)
    print(f"saved {output_path}")


if __name__ == "__main__":
    plot_orbit()
