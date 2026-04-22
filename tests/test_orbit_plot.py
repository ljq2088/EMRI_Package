from pathlib import Path
import sys

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
from plot_orbit_3d import plot_orbit


def main():
    output = Path("orbit_3d_plot.png")
    if output.exists():
        output.unlink()

    plot_orbit(output_path=output, duration=200.0, dt=1.0)

    assert output.exists(), "plot file was not created"
    assert output.stat().st_size > 0, "plot file is empty"
    print(f"plot test passed: {output} ({output.stat().st_size} bytes)")


if __name__ == "__main__":
    main()
