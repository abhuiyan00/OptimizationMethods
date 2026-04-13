"""
PFSP Experiment Plotter
=======================
Run from your project root (where results/ folder is):
    pip install pandas matplotlib
    python generate_plots.py

Generates PNG plots in results/plots/
"""

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os
import glob

RESULTS_DIR = "results"
PLOTS_DIR = os.path.join(RESULTS_DIR, "plots")
os.makedirs(PLOTS_DIR, exist_ok=True)

# Consistent colors for methods
COLORS = {
    "Greedy": "#888888",
    "RandomSearch": "#e67e22",
    "GA": "#2ecc71",
    "TS": "#3498db",
}
METHOD_ORDER = ["Greedy", "RandomSearch", "GA", "TS"]

# ============================================================
# 1. CONVERGENCE PLOTS (history files) — one per instance
# ============================================================
def plot_convergence():
    instances = ["tai20_5_0", "tai20_10_0", "tai20_20_0",
                 "tai100_10_0", "tai100_20_0", "tai500_20_0"]

    for inst in instances:
        fig, ax = plt.subplots(figsize=(8, 5))
        has_data = False

        for method in ["GA", "TS", "RandomSearch"]:
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

        # Add greedy as horizontal line
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


# ============================================================
# 2. COMPARISON BAR CHART — best fitness per method per instance
# ============================================================
def plot_comparison_bars():
    summary = pd.read_csv(os.path.join(RESULTS_DIR, "summary.csv"))

    instances = ["tai20_5_0", "tai20_10_0", "tai20_20_0",
                 "tai100_10_0", "tai100_20_0", "tai500_20_0"]

    # Compute average best fitness per method per instance
    agg = summary.groupby(["instance", "method"]).agg(
        avgBest=("bestFitness", "mean"),
        minBest=("bestFitness", "min"),
        stdBest=("bestFitness", "std"),
    ).reset_index()
    agg["stdBest"] = agg["stdBest"].fillna(0)

    # --- One chart per difficulty group ---
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


# ============================================================
# 3. SUMMARY TABLE (printed + saved as CSV)
# ============================================================
def make_summary_table():
    summary = pd.read_csv(os.path.join(RESULTS_DIR, "summary.csv"))

    table = summary.groupby(["instance", "method"]).agg(
        best=("bestFitness", "min"),
        avgBest=("bestFitness", "mean"),
        std=("bestFitness", "std"),
        avgTime=("timeMs", "mean"),
        evals=("evaluations", "first"),
    ).reset_index()
    table["std"] = table["std"].fillna(0)
    table = table.round({"avgBest": 1, "std": 1, "avgTime": 1})

    out = os.path.join(PLOTS_DIR, "summary_table.csv")
    table.to_csv(out, index=False)
    print(f"  ✓ summary_table.csv")
    print(table.to_string(index=False))


# ============================================================
# 4. PARAMETER SWEEP PLOTS
# ============================================================
def plot_param_sweeps():
    param_files = glob.glob(os.path.join(RESULTS_DIR, "params_*.csv"))

    for pf in param_files:
        df = pd.read_csv(pf)
        inst = os.path.basename(pf).replace("params_", "").replace(".csv", "")

        params = df["paramName"].unique()
        for param in params:
            sub = df[df["paramName"] == param].sort_values("paramValue")
            if sub["avgFitness"].nunique() <= 1:
                # Skip if all identical (old buggy data)
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


# ============================================================
# 5. EXECUTION TIME COMPARISON
# ============================================================
def plot_time_comparison():
    summary = pd.read_csv(os.path.join(RESULTS_DIR, "summary.csv"))
    agg = summary.groupby(["instance", "method"])["timeMs"].mean().reset_index()

    instances = ["tai20_5_0", "tai20_10_0", "tai20_20_0",
                 "tai100_10_0", "tai100_20_0", "tai500_20_0"]

    fig, ax = plt.subplots(figsize=(10, 5))
    x = np.arange(len(instances))
    width = 0.18

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


# ============================================================
# RUN ALL
# ============================================================
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