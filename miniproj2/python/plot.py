import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from prefixed import Float

# 1. Parse and load the data

df = pd.read_csv("../results.csv")

# Calculate mean CPU time and mean Reorder count for overlapping tests
df_agg = df.groupby(["Device", "Type", "Barrier"]).agg({"CPU": "mean", "Reorder": "sum", "Iterations":"sum"}).reset_index()

# 2. Sort categories logically
barrier_order = ["Without", "Partial", "Full"]
df_agg["Barrier"] = pd.Categorical(df_agg["Barrier"], categories=barrier_order, ordered=True)
df_agg = df_agg.sort_values(["Device", "Type", "Barrier"])

devices = df_agg["Device"].unique()
types = ["none", "separated"]

# 3. Create subplots for Grouping
fig, axes = plt.subplots(len(devices), len(types), figsize=(12, 4 * len(devices)), sharey=False)
fig.suptitle("Benchmark: średni czas operacji i prawdopodobieństwo zmian kolejności", fontsize=18, fontweight='bold')

colors = ['#4FA4D9FF', '#f0ad4e', '#874FD9FF']

# 4. Draw charts
for i, device in enumerate(devices):
    for j, b_type in enumerate(types):
        ax = axes[i, j]
        
        # Filter subgroup subset
        subset = df_agg[(df_agg["Device"] == device) & (df_agg["Type"] == b_type)]
        
        if subset.empty:
            ax.axis("off") # hide empty subplots if data is somehow missing
            continue
            
        x = np.arange(len(barrier_order))
        ax.grid(axis='y', color='grey')
        
        # CPU Bars
        bars = ax.bar(x, subset["CPU"], width=0.6, color=colors, edgecolor='black', zorder=5)
        
        ax.set_title(f"Procesor: {device} | Wariant : {"separated templates" if b_type=='separated' else "if"}", fontsize=12)
        ax.set_ylabel("Średni czas CPU [ns] (mniej = lepiej)", fontsize=10)
        ax.set_xticks(x)
        ax.set_xticklabels(barrier_order, fontsize=10)
        
        # Max margin for reorder labels dynamically
        max_height = subset["CPU"].max()
        ax.set_ylim(0,800)
        
        # 5. Add value labels (CPU & Reorder count)
        for bar, cpu, reorder, iters in zip(bars, subset["CPU"], subset["Reorder"], subset["Iterations"]):
            height = bar.get_height()
            
            # Label: CPU Time Inside the bar
            ax.text(bar.get_x() + bar.get_width()/2, height / 2,
                    f'{cpu:.0f} ns\nN={Float(iters):.4H}', ha='center', va='center', color='white', 
                    fontweight='bold', fontsize=11, zorder=10)
            
            # Label: Reorder Count Above the bar
            if reorder > 0 and iters > 0:
                reorder_text = f"Reorders:\n{reorder}"
                ax.text(bar.get_x() + bar.get_width()/2, height + (max_height * 0.05),
                        reorder_text, ha='center', va='bottom', color='red', 
                        fontweight='bold', fontsize=10)
            else:
                 ax.text(bar.get_x() + bar.get_width()/2, height + (max_height * 0.05),
                        "Reorders:\n0", ha='center', va='bottom', color='green', 
                        fontweight='bold', fontsize=10)

plt.tight_layout(rect=[0, 0.02, 1, 0.98])
plt.savefig("../img.png", dpi=300)