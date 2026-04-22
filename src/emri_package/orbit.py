from dataclasses import dataclass

import numpy as np

from ._orbit import (
    BabakNKOrbitCPP,
    compute_fundamental_frequencies_cpp,
    compute_fundamental_frequencies_from_pei_constants_cpp,
    compute_fundamental_frequencies_from_pei_cpp,
    get_conserved_quantities_cpp,
)


@dataclass
class OrbitTrajectory:
    t: np.ndarray
    p: np.ndarray
    e: np.ndarray
    iota: np.ndarray
    psi: np.ndarray
    chi: np.ndarray
    phi: np.ndarray
    r: np.ndarray
    theta: np.ndarray


@dataclass
class OrbitKinematicsTrajectory:
    t: np.ndarray
    p: np.ndarray
    e: np.ndarray
    iota: np.ndarray
    psi: np.ndarray
    chi: np.ndarray
    phi: np.ndarray
    r: np.ndarray
    theta: np.ndarray
    E: np.ndarray
    Lz: np.ndarray
    Q: np.ndarray
    ut: np.ndarray
    ur: np.ndarray
    utheta: np.ndarray
    uphi: np.ndarray
    sigma: np.ndarray
    delta: np.ndarray
    P: np.ndarray


def bl_to_cartesian(r: np.ndarray, theta: np.ndarray, phi: np.ndarray):
    x = r * np.sin(theta) * np.cos(phi)
    y = r * np.sin(theta) * np.sin(phi)
    z = r * np.cos(theta)
    return x, y, z


class KerrOrbit:
    """High-level wrapper over the extracted NK/Kerr orbit backend.

    Use `mu=0.0` for pure geodesic evolution.
    """

    def __init__(self, M: float, a: float, p: float, e: float, iota: float, mu: float = 0.0):
        self.M = M
        self.a = a
        self.p = p
        self.e = e
        self.iota = iota
        self.mu = mu
        self._backend = BabakNKOrbitCPP(M, a, p, e, iota, mu)

    def conserved_quantities(self):
        return get_conserved_quantities_cpp(self.M, self.a, self.p, self.e, self.iota)

    def fundamental_frequencies(self):
        consts = self.conserved_quantities()
        return compute_fundamental_frequencies_from_pei_constants_cpp(
            self.a, self.p, self.e, self.iota, consts.E, consts.Lz, consts.Q
        )

    def current_kinematics(self):
        return self._backend.get_current_kinematics()

    def state_kinematics(self, state):
        return self._backend.get_state_kinematics(state)

    def evolve(self, duration: float, dt: float) -> OrbitTrajectory:
        states = self._backend.evolve(duration, dt)
        if not states:
            return OrbitTrajectory(*(np.array([]) for _ in range(9)))
        return OrbitTrajectory(
            t=np.array([s.t for s in states]),
            p=np.array([s.p for s in states]),
            e=np.array([s.e for s in states]),
            iota=np.array([s.iota for s in states]),
            psi=np.array([s.psi for s in states]),
            chi=np.array([s.chi for s in states]),
            phi=np.array([s.phi for s in states]),
            r=np.array([s.r for s in states]),
            theta=np.array([s.theta for s in states]),
        )

    def evolve_with_kinematics(self, duration: float, dt: float) -> tuple[OrbitTrajectory, OrbitKinematicsTrajectory]:
        states = self._backend.evolve(duration, dt)
        if not states:
            empty9 = [np.array([]) for _ in range(9)]
            empty19 = [np.array([]) for _ in range(19)]
            return OrbitTrajectory(*empty9), OrbitKinematicsTrajectory(*empty19)

        traj = OrbitTrajectory(
            t=np.array([s.t for s in states]),
            p=np.array([s.p for s in states]),
            e=np.array([s.e for s in states]),
            iota=np.array([s.iota for s in states]),
            psi=np.array([s.psi for s in states]),
            chi=np.array([s.chi for s in states]),
            phi=np.array([s.phi for s in states]),
            r=np.array([s.r for s in states]),
            theta=np.array([s.theta for s in states]),
        )
        kin_states = [self._backend.get_state_kinematics(s) for s in states]
        kin = OrbitKinematicsTrajectory(
            t=np.array([k.t for k in kin_states]),
            p=np.array([k.p for k in kin_states]),
            e=np.array([k.e for k in kin_states]),
            iota=np.array([k.iota for k in kin_states]),
            psi=np.array([k.psi for k in kin_states]),
            chi=np.array([k.chi for k in kin_states]),
            phi=np.array([k.phi for k in kin_states]),
            r=np.array([k.r for k in kin_states]),
            theta=np.array([k.theta for k in kin_states]),
            E=np.array([k.E for k in kin_states]),
            Lz=np.array([k.Lz for k in kin_states]),
            Q=np.array([k.Q for k in kin_states]),
            ut=np.array([k.ut for k in kin_states]),
            ur=np.array([k.ur for k in kin_states]),
            utheta=np.array([k.utheta for k in kin_states]),
            uphi=np.array([k.uphi for k in kin_states]),
            sigma=np.array([k.sigma for k in kin_states]),
            delta=np.array([k.delta for k in kin_states]),
            P=np.array([k.P for k in kin_states]),
        )
        return traj, kin


def fundamental_frequencies_from_constants(a: float, E: float, Lz: float, Q: float):
    return compute_fundamental_frequencies_cpp(a, E, Lz, Q)
