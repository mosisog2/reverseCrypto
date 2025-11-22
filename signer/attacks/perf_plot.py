#!/usr/bin/env python3
"""Benchmark CRT vs. full signing and visualize the results."""

from __future__ import annotations

import csv
import pathlib
import subprocess
from collections import defaultdict

import matplotlib.pyplot as plt


def run_benchmark(repo_root: pathlib.Path) -> list[dict[str, str]]:
    attacks_dir = repo_root / "attacks"
    subprocess.run(["make", "-C", str(repo_root), "attacks/perf_benchmark"], check=True)

    binary = attacks_dir / "perf_benchmark"
    result = subprocess.run([str(binary)], check=True, capture_output=True, text=True)

    rows: list[dict[str, str]] = []
    reader = csv.DictReader(result.stdout.strip().splitlines())
    for row in reader:
        rows.append(row)
    (attacks_dir / "perf_results.csv").write_text(result.stdout)
    return rows


def plot_results(rows: list[dict[str, str]], repo_root: pathlib.Path) -> None:
    methods = [
        "full",
        "crt",
        "crt+post_verify",
        "crt+double",
        "crt+exp_blind",
        "crt+msg_blind",
    ]

    palette = {
        "full": "#1f77b4",
        "crt": "#2ca02c",
        "crt+post_verify": "#ff7f0e",
        "crt+double": "#d62728",
        "crt+exp_blind": "#9467bd",
        "crt+msg_blind": "#8c564b",
    }

    series: dict[str, list[tuple[int, float]]] = defaultdict(list)
    for row in rows:
        bits = int(row["bits"])
        method = row["method"]
        time_ms = float(row["time_ms"])
        if method in palette:
            series[method].append((bits, time_ms))

    plt.figure(figsize=(9, 5))
    for method in methods:
        data = sorted(series.get(method, []))
        if not data:
            continue
        xs, ys = zip(*data)
        plt.plot(xs, ys, marker="o", color=palette[method], label=method)

    plt.xlabel("Modulus bit length")
    plt.ylabel("Average signing time (ms)")
    plt.title("RSA signing performance: CRT vs. full and defenses")
    plt.grid(True, linestyle="--", linewidth=0.5, alpha=0.5)
    plt.legend()
    output_path = repo_root / "attacks" / "perf_timing.png"
    plt.tight_layout()
    plt.savefig(output_path, dpi=150)
    print(f"Saved plot to {output_path}")

    # Print overhead summary relative to CRT baseline for convenience.
    print("\nOverhead relative to baseline CRT:")
    by_key = defaultdict(dict)
    for row in rows:
        by_key[(row["key"], row["bits"])][row["method"]] = float(row["rel_to_crt"])

    for (key, bits), measures in sorted(by_key.items(), key=lambda item: int(item[0][1])):
        print(f"  {key} (bits={bits}):")
        base = measures.get("crt", 1.0)
        for method in methods:
            if method == "crt":
                continue
            ratio = measures.get(method)
            if ratio is None:
                continue
            print(f"    {method:18s}: {ratio:.3f}x")


def main() -> None:
    repo_root = pathlib.Path(__file__).resolve().parents[1]
    rows = run_benchmark(repo_root)
    plot_results(rows, repo_root)
    plt.show()


if __name__ == "__main__":
    main()
