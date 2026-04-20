"""
PFSP plot generator.

Run from the project root:
    pip install pandas matplotlib
    python generate_plots.py

Saves PNG files under results/plots/.
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import glob
import re

RESULTS_DIR = "results"
PLOTS_DIR = os.path.join(RESULTS_DIR, "plots")
os.makedirs(PLOTS_DIR, exist_ok=True)

# Fixed color map for method names.
COLORS = {
    "Greedy": "#888888",
    "RandomSearch": "#e67e22",
    "GA": "#2ecc71",
    "TS": "#3498db",
    "ILS": "#8e44ad",
}
METHOD_ORDER = ["Greedy", "RandomSearch", "GA", "TS", "ILS"]


def parse_instance_shape(instance_name):
    match = re.match(r"tai(\d+)_(\d+)_\d+", instance_name)
    if not match:
        return None, None
    return int(match.group(1)), int(match.group(2))

# 1) Convergence plots from history_*.csv files.
def plot_convergence():
    instances = ["tai20_5_0", "tai20_10_0", "tai20_20_0",
                 "tai100_10_0", "tai100_20_0", "tai500_20_0"]

    for inst in instances:
        fig, ax = plt.subplots(figsize=(8, 5))
        has_data = False

        for method in ["GA", "TS", "RandomSearch", "ILS"]:
            path = os.path.join(RESULTS_DIR, f"history_{method}_{inst}_rep0.csv")
            if not os.path.exists(path):
                continue
            df = pd.read_csv(path)
            if df.empty or len(df) < 2:
                continue
            has_data = True
            ax.plot(df["generation"], df["best"], label=f"{method} best",
                    color=COLORS.get(method, "black"), linewidth=1.5)
            ax.plot(df["generation"], df["average"], label=f"{method} avg",
                    color=COLORS.get(method, "black"), linewidth=1, linestyle="--", alpha=0.6)

        # Greedy has one point, so draw it as a horizontal baseline.
        greedy_path = os.path.join(RESULTS_DIR, f"history_Greedy_{inst}_rep0.csv")
        if os.path.exists(greedy_path):
            gdf = pd.read_csv(greedy_path)
            if not gdf.empty:
                ax.axhline(y=gdf["best"].iloc[0], color=COLORS["Greedy"],
                           linestyle=":", linewidth=1.5, label="Greedy")
                has_data = True

        if has_data:
            ax.set_xlabel("Generation / Iteration")
            ax.set_ylabel("Total Flow Time")
            ax.set_title(f"Convergence — {inst}")
            ax.legend(fontsize=8)
            ax.grid(True, alpha=0.3)
            fig.tight_layout()
            fig.savefig(os.path.join(PLOTS_DIR, f"convergence_{inst}.png"), dpi=150)
            print(f"  ✓ convergence_{inst}.png")
        plt.close(fig)


# 2) Method comparison bars (avg best fitness).
def plot_comparison_bars():
    summary = pd.read_csv(os.path.join(RESULTS_DIR, "summary.csv"))

    instances = ["tai20_5_0", "tai20_10_0", "tai20_20_0",
                 "tai100_10_0", "tai100_20_0", "tai500_20_0"]

    # Aggregate repeated runs.
    agg = summary.groupby(["instance", "method"]).agg(
        avgBest=("bestFitness", "mean"),
        minBest=("bestFitness", "min"),
        stdBest=("bestFitness", "std"),
    ).reset_index()
    agg["stdBest"] = agg["stdBest"].fillna(0)

    # One chart per instance size group.
    groups = {
        "Easy (20 jobs)": ["tai20_5_0", "tai20_10_0", "tai20_20_0"],
        "Medium (100 jobs)": ["tai100_10_0", "tai100_20_0"],
        "Hard (500 jobs)": ["tai500_20_0"],
    }

    for title, insts in groups.items():
        sub = agg[agg["instance"].isin(insts)]
        if sub.empty:
            continue

        fig, ax = plt.subplots(figsize=(max(6, len(insts) * 3), 5))
        x = np.arange(len(insts))
        width = 0.18

        for i, method in enumerate(METHOD_ORDER):
            mdata = sub[sub["method"] == method]
            vals, errs = [], []
            for inst in insts:
                row = mdata[mdata["instance"] == inst]
                vals.append(row["avgBest"].values[0] if len(row) else 0)
                errs.append(row["stdBest"].values[0] if len(row) else 0)
            ax.bar(x + i * width, vals, width, yerr=errs, label=method,
                   color=COLORS.get(method, "gray"), capsize=3)

        ax.set_xticks(x + width * 1.5)
        ax.set_xticklabels(insts, fontsize=9)
        ax.set_ylabel("Avg Best Fitness (lower = better)")
        ax.set_title(f"Method Comparison — {title}")
        ax.legend()
        ax.grid(True, axis="y", alpha=0.3)
        fig.tight_layout()
        fname = f"comparison_{title.split('(')[0].strip().lower()}.png"
        fig.savefig(os.path.join(PLOTS_DIR, fname), dpi=150)
        print(f"  ✓ {fname}")
        plt.close(fig)


# 3) Summary table (printed and saved).
def make_summary_table():
    summary = pd.read_csv(os.path.join(RESULTS_DIR, "summary.csv"))

    table = summary.groupby(["instance", "method"], as_index=False).agg(
        best_found=("bestFitness", "min"),
        avg_found=("bestFitness", "mean"),
        worst_found=("bestFitness", "max"),
        std_dev=("bestFitness", "std"),
        avg_time_s=("timeMs", lambda s: s.mean() / 1000.0),
        evals=("evaluations", "mean"),
    )

    table["std_dev"] = table["std_dev"].fillna(0.0)
    table["rpd_avg_%"] = ((table["avg_found"] - table["best_found"]) /
                           table["best_found"] * 100.0)
    table["stability_cv_%"] = np.where(
        table["avg_found"] != 0,
        table["std_dev"] / table["avg_found"] * 100.0,
        0.0,
    )

    table[["jobs", "mc"]] = table["instance"].apply(
        lambda inst: pd.Series(parse_instance_shape(inst))
    )

    table = table[[
        "instance",
        "method",
        "jobs",
        "mc",
        "best_found",
        "avg_found",
        "worst_found",
        "std_dev",
        "rpd_avg_%",
        "stability_cv_%",
        "avg_time_s",
        "evals",
    ]]

    table = table.round({
        "avg_found": 1,
        "worst_found": 1,
        "std_dev": 1,
        "rpd_avg_%": 2,
        "stability_cv_%": 2,
        "avg_time_s": 3,
        "evals": 0,
    })
    table["evals"] = table["evals"].astype(int)

    out = os.path.join(PLOTS_DIR, "summary_table.csv")
    table.to_csv(out, index=False)
    print(f"  ✓ summary_table.csv")
    print(table.to_string(index=False))


# 4) Parameter-sweep sensitivity plots.
def plot_param_sweeps():
    param_files = glob.glob(os.path.join(RESULTS_DIR, "params_*.csv"))

    for pf in param_files:
        df = pd.read_csv(pf)
        inst = os.path.basename(pf).replace("params_", "").replace(".csv", "")

        params = df["paramName"].unique()
        for param in params:
            sub = df[df["paramName"] == param].sort_values("paramValue")
            if sub["avgFitness"].nunique() <= 1:
                # Skip flat curves from stale/invalid data.
                print(f"  ⚠ Skipping {param} for {inst} (all values identical — re-run after fix)")
                continue

            fig, ax = plt.subplots(figsize=(7, 4.5))
            ax.errorbar(sub["paramValue"], sub["avgFitness"], yerr=sub["stdDev"],
                        marker="o", capsize=4, color="#2ecc71", linewidth=1.5)
            ax.set_xlabel(param)
            ax.set_ylabel("Avg Fitness (lower = better)")
            ax.set_title(f"Parameter Sensitivity: {param} — {inst}")
            ax.grid(True, alpha=0.3)
            fig.tight_layout()
            fname = f"param_{param}_{inst}.png"
            fig.savefig(os.path.join(PLOTS_DIR, fname), dpi=150)
            print(f"  ✓ {fname}")
            plt.close(fig)


# 5) Runtime comparison.
def plot_time_comparison():
    summary = pd.read_csv(os.path.join(RESULTS_DIR, "summary.csv"))
    agg = summary.groupby(["instance", "method"])["timeMs"].mean().reset_index()

    instances = ["tai20_5_0", "tai20_10_0", "tai20_20_0",
                 "tai100_10_0", "tai100_20_0", "tai500_20_0"]

    fig, ax = plt.subplots(figsize=(11, 5))
    x = np.arange(len(instances))
    width = 0.15

    for i, method in enumerate(METHOD_ORDER):
        vals = []
        for inst in instances:
            row = agg[(agg["instance"] == inst) & (agg["method"] == method)]
            vals.append(row["timeMs"].values[0] if len(row) else 0)
        ax.bar(x + i * width, vals, width, label=method,
               color=COLORS.get(method, "gray"))

    ax.set_xticks(x + width * 1.5)
    ax.set_xticklabels(instances, fontsize=8, rotation=15)
    ax.set_ylabel("Avg Time (ms)")
    ax.set_title("Execution Time Comparison")
    ax.legend()
    ax.set_yscale("log")
    ax.grid(True, axis="y", alpha=0.3)
    fig.tight_layout()
    fig.savefig(os.path.join(PLOTS_DIR, "time_comparison.png"), dpi=150)
    print(f"  ✓ time_comparison.png")
    plt.close(fig)


# Run all plotting steps.
if __name__ == "__main__":
    print("=" * 50)
    print("PFSP Experiment Plot Generator")
    print("=" * 50)

    print("\n[1/5] Convergence plots...")
    plot_convergence()

    print("\n[2/5] Comparison bar charts...")
    plot_comparison_bars()

    print("\n[3/5] Summary table...")
    make_summary_table()

    print("\n[4/5] Parameter sweep plots...")
    plot_param_sweeps()

    print("\n[5/5] Time comparison...")
    plot_time_comparison()

    print(f"\nAll plots saved to {PLOTS_DIR}/")
    print("Import these PNGs into your report.")