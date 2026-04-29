"""
Analyse des résultats de clustering
Usage : python analyse.py [resultats.csv]
"""
import sys
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
from pathlib import Path

# ── Style ─────────────────────────────────────────────────────────────────────
BG      = "#0f1117"
SURFACE = "#1a1d27"
BORDER  = "#2a2d3e"
TEXT    = "#e2e8f0"
MUTED   = "#64748b"

C_BLUE   = "#4f8ef7"
C_GREEN  = "#34d399"
C_ORANGE = "#fb923c"
C_RED    = "#f87171"
C_PURPLE = "#a78bfa"
C_YELLOW = "#fbbf24"

STRAT_COLORS = {
    "Pessimiste":  C_BLUE,
    "Optimiste":   C_GREEN,
    "Overbooking": C_ORANGE,
}
CONFIG_COLORS = {
    "N/A":          "#94a3b8",
    "None":         C_BLUE,
    "Centroid":     C_GREEN,
    "BestUser":     C_RED,
    "Mean(Local)":  C_PURPLE,
    "Mean(Global)": C_YELLOW,
}

plt.rcParams.update({
    "figure.facecolor": BG, "axes.facecolor": SURFACE,
    "axes.edgecolor": BORDER, "axes.labelcolor": TEXT,
    "axes.titlecolor": TEXT, "xtick.color": MUTED, "ytick.color": MUTED,
    "text.color": TEXT, "grid.color": BORDER, "grid.linestyle": "--",
    "grid.alpha": 0.5, "legend.facecolor": SURFACE, "legend.edgecolor": BORDER,
    "font.family": "monospace", "font.size": 10,
})

def savefig(fig, path):
    fig.savefig(path, dpi=150, bbox_inches="tight", facecolor=BG)
    plt.close(fig)
    print(f"  ✓ {Path(path).name}")

def styled(ax, title, xlabel="", ylabel="", grid_axis="y"):
    ax.set_title(title, fontsize=11, fontweight="bold", pad=10)
    ax.set_xlabel(xlabel, labelpad=6)
    ax.set_ylabel(ylabel, labelpad=6)
    ax.spines[["top","right"]].set_visible(False)
    if grid_axis:
        ax.grid(axis=grid_axis, zorder=0)


# ══════════════════════════════════════════════════════════════════════════════
# 1. TEMPS D'EXÉCUTION — Greedy vs Quadtree (log scale)
# ══════════════════════════════════════════════════════════════════════════════
def plot_temps(df, out):
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle("Temps d'exécution : Greedy vs Quadtree", fontsize=13, fontweight="bold")

    # Gauche : comparaison directe par stratégie (log)
    ax = axes[0]
    greedy = df[df["Algo"] == "Greedy"].groupby("Strategie")["Temps(ms)"].mean()
    quad   = df[(df["Algo"] == "Quadtree") & (df["Post-Trait"] == "None")]\
               .groupby("Strategie")["Temps(ms)"].mean()
    strats = greedy.index.tolist()
    x = np.arange(len(strats))
    b1 = ax.bar(x - 0.2, greedy.values / 1000, 0.38, label="Greedy",
                color=C_RED, alpha=0.85, zorder=3)
    b2 = ax.bar(x + 0.2, quad.values / 1000, 0.38, label="Quadtree (None)",
                color=C_BLUE, alpha=0.85, zorder=3)
    ax.set_yscale("log")
    ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda v, _: f"{v:.0f}s" if v >= 1 else f"{v*1000:.0f}ms"))
    for bar in list(b1) + list(b2):
        h = bar.get_height()
        label = f"{h:.0f}s" if h >= 1 else f"{h*1000:.0f}ms"
        ax.text(bar.get_x() + bar.get_width()/2, h*1.15, label,
                ha="center", va="bottom", fontsize=8, color=MUTED)
    styled(ax, "Comparaison directe (échelle log)", ylabel="Temps (log)")
    ax.set_xticks(x); ax.set_xticklabels(strats)
    ax.legend()

    # Droite : temps Quadtree uniquement, par Post-Trait
    ax = axes[1]
    qt = df[df["Algo"] == "Quadtree"].groupby("Post-Trait")["Temps(ms)"].mean()
    configs = qt.index.tolist()
    colors  = [CONFIG_COLORS.get(c, MUTED) for c in configs]
    bars = ax.bar(configs, qt.values, color=colors, alpha=0.85, zorder=3)
    for bar in bars:
        h = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2, h + 1,
                f"{h:.0f}ms", ha="center", va="bottom", fontsize=8, color=MUTED)
    styled(ax, "Quadtree : temps par Post-Traitement", ylabel="Temps moyen (ms)")
    ax.set_xticklabels(configs, rotation=15, ha="right")

    fig.tight_layout()
    savefig(fig, out / "1_temps_execution.png")


# ══════════════════════════════════════════════════════════════════════════════
# 2. NOMBRE DE CLUSTERS — impact du Post-Traitement
# ══════════════════════════════════════════════════════════════════════════════
def plot_clusters(df, out):
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle("Nombre de clusters par Post-Traitement", fontsize=13, fontweight="bold")

    for idx, (hilbert, label) in enumerate([(False, "Sans Hilbert"), (True, "Avec Hilbert")]):
        ax = axes[idx]
        sub = df[(df["Algo"] == "Quadtree") & (df["Hilbert"] == hilbert)]
        configs = [c for c in ["None","Centroid","BestUser","Mean(Local)","Mean(Global)"]
                   if c in sub["Post-Trait"].values]
        strats  = ["Pessimiste","Optimiste","Overbooking"]
        x = np.arange(len(configs))
        w = 0.25
        for i, strat in enumerate(strats):
            vals = [sub[(sub["Post-Trait"]==c) & (sub["Strategie"]==strat)]["Clusters"].mean()
                    for c in configs]
            ax.bar(x + i*w, vals, w, label=strat,
                   color=STRAT_COLORS[strat], alpha=0.85, zorder=3)
        # Ligne de référence Greedy
        greedy_mean = df[df["Algo"]=="Greedy"]["Clusters"].mean()
        ax.axhline(greedy_mean, color=C_RED, linestyle=":", linewidth=1.5,
                   label=f"Greedy (~{greedy_mean:.0f})")
        styled(ax, label, ylabel="Nombre de clusters")
        ax.set_xticks(x + w)
        ax.set_xticklabels(configs, rotation=15, ha="right", fontsize=9)
        ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda v,_: f"{v/1000:.0f}k"))
        ax.legend(fontsize=8)

    fig.tight_layout()
    savefig(fig, out / "2_nb_clusters.png")


# ══════════════════════════════════════════════════════════════════════════════
# 3. DISTANCE MOYENNE — quel Post-Traitement rapproche les users ?
# ══════════════════════════════════════════════════════════════════════════════
def plot_distance(df, out):
    fig, ax = plt.subplots(figsize=(12, 5))

    qt = df[df["Algo"] == "Quadtree"]
    configs = ["None","Centroid","BestUser","Mean(Local)","Mean(Global)"]
    x = np.arange(len(configs))
    w = 0.18

    for i, (hilbert, marker) in enumerate([(False, "//"), (True, "")]):
        sub = qt[qt["Hilbert"] == hilbert]
        for j, strat in enumerate(["Pessimiste","Optimiste","Overbooking"]):
            vals = [sub[(sub["Post-Trait"]==c) & (sub["Strategie"]==strat)]["Dist_Moy(km)"].mean()
                    for c in configs]
            offset = (i * 3 + j - 2.5) * w * 0.48
            label = f"{strat} {'(H)' if hilbert else ''}"
            ax.bar(x + offset, vals, w * 0.9,
                   color=STRAT_COLORS[strat],
                   alpha=0.9 if not hilbert else 0.55,
                   hatch=marker, zorder=3, label=label)

    # Ligne Greedy
    greedy_dist = df[df["Algo"]=="Greedy"]["Dist_Moy(km)"].mean()
    ax.axhline(greedy_dist, color=C_RED, linestyle=":", linewidth=1.5,
               label=f"Greedy ({greedy_dist:.1f} km)")

    styled(ax, "Distance moyenne au centre du cluster par Post-Traitement",
           ylabel="Distance (km)")
    ax.set_xticks(x)
    ax.set_xticklabels(configs, fontsize=10)
    ax.set_ylim(15, 28)

    # Annotations min/max
    ax.annotate("← Centroid = meilleure\ncompacité géographique",
                xy=(1, 19.0), xytext=(1.3, 16.5),
                arrowprops=dict(arrowstyle="->", color=C_GREEN),
                color=C_GREEN, fontsize=8)
    ax.annotate("BestUser = pire\ndistance →",
                xy=(2, 25.5), xytext=(2.5, 26.5),
                arrowprops=dict(arrowstyle="->", color=C_RED),
                color=C_RED, fontsize=8)

    ax.legend(fontsize=7, ncol=3, loc="upper left")
    fig.tight_layout()
    savefig(fig, out / "3_distance_post_trait.png")


# ══════════════════════════════════════════════════════════════════════════════
# 4. TAUX DE REMPLISSAGE — par stratégie, tous algos confondus
# ══════════════════════════════════════════════════════════════════════════════
def plot_remplissage(df, out):
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle("Taux de remplissage des clusters", fontsize=13, fontweight="bold")

    # Gauche : par stratégie (toutes configs)
    ax = axes[0]
    for strat, color in STRAT_COLORS.items():
        vals = df[df["Strategie"] == strat]["Rempli(%)"].values
        ax.barh(strat, vals.mean(), color=color, alpha=0.85, zorder=3)
        ax.errorbar(vals.mean(), strat, xerr=vals.std(),
                    fmt="none", color="white", capsize=4, linewidth=1.5)
        ax.text(vals.mean() + 0.3, strat, f"{vals.mean():.1f}%",
                va="center", fontsize=9, color=MUTED)
    styled(ax, "Remplissage moyen par stratégie\n(± écart-type)",
           xlabel="Remplissage (%)", grid_axis="x")
    ax.spines[["top","right"]].set_visible(False)
    ax.set_xlim(0, 35)

    # Droite : impact Cap% 80 vs 100 par stratégie
    ax = axes[1]
    strats = ["Pessimiste","Optimiste","Overbooking"]
    x = np.arange(len(strats))
    for i, cap in enumerate([80, 100]):
        vals = [df[(df["Strategie"]==s) & (df["Cap%"]==cap)]["Rempli(%)"].mean()
                for s in strats]
        bars = ax.bar(x + i*0.35 - 0.18, vals, 0.34,
                      label=f"Cap {cap}%",
                      color=C_BLUE if cap == 80 else C_ORANGE,
                      alpha=0.85, zorder=3)
        for bar, v in zip(bars, vals):
            ax.text(bar.get_x() + bar.get_width()/2, bar.get_height() + 0.2,
                    f"{v:.1f}%", ha="center", va="bottom", fontsize=8, color=MUTED)
    styled(ax, "Impact de Cap% sur le remplissage", ylabel="Remplissage (%)")
    ax.set_xticks(x); ax.set_xticklabels(strats)
    ax.legend()

    fig.tight_layout()
    savefig(fig, out / "4_remplissage.png")


# ══════════════════════════════════════════════════════════════════════════════
# 5. IMPACT HILBERT — clusters & distance côte à côte
# ══════════════════════════════════════════════════════════════════════════════
def plot_hilbert(df, out):
    fig, axes = plt.subplots(1, 2, figsize=(14, 5))
    fig.suptitle("Impact de la courbe de Hilbert (Quadtree uniquement)",
                 fontsize=13, fontweight="bold")

    qt = df[df["Algo"] == "Quadtree"]
    configs = ["None","Centroid","BestUser","Mean(Local)","Mean(Global)"]

    for ax, metric, ylabel in zip(axes,
                                   ["Clusters", "Dist_Moy(km)"],
                                   ["Nombre de clusters", "Distance moyenne (km)"]):
        x = np.arange(len(configs))
        for i, (hilbert, label, color) in enumerate([
            (False, "Sans Hilbert", C_BLUE),
            (True,  "Avec Hilbert",  C_GREEN),
        ]):
            sub = qt[qt["Hilbert"] == hilbert]
            vals = [sub[sub["Post-Trait"]==c][metric].mean() for c in configs]
            ax.bar(x + i*0.35 - 0.18, vals, 0.34, label=label,
                   color=color, alpha=0.85, zorder=3)

        styled(ax, ylabel, ylabel=ylabel)
        ax.set_xticks(x)
        ax.set_xticklabels(configs, rotation=15, ha="right", fontsize=9)
        if metric == "Clusters":
            ax.yaxis.set_major_formatter(ticker.FuncFormatter(lambda v,_: f"{v/1000:.0f}k"))
        ax.legend()

    fig.tight_layout()
    savefig(fig, out / "5_impact_hilbert.png")


# ══════════════════════════════════════════════════════════════════════════════
# 6. SYNTHÈSE — radar multi-critères (Quadtree, sans Hilbert)
# ══════════════════════════════════════════════════════════════════════════════
def plot_radar(df, out):
    sub = df[(df["Algo"] == "Quadtree") & (~df["Hilbert"])].copy()

    metrics_def = {
        "Rempli(%)":    True,   # plus grand = mieux
        "Dist_Moy(km)": False,  # plus petit = mieux
        "Temps(ms)":    False,
        "Clusters":     False,
    }
    labels_r = ["Remplissage\n(↑)", "Distance\n(↓)", "Temps\n(↓)", "Nb Clusters\n(↓)"]

    grouped = sub.groupby("Post-Trait")[list(metrics_def.keys())].mean()
    norm = grouped.copy()
    for col, higher_better in metrics_def.items():
        mn, mx = grouped[col].min(), grouped[col].max()
        if mx == mn:
            norm[col] = 1.0
        elif higher_better:
            norm[col] = (grouped[col] - mn) / (mx - mn)
        else:
            norm[col] = 1 - (grouped[col] - mn) / (mx - mn)

    N      = len(metrics_def)
    angles = np.linspace(0, 2*np.pi, N, endpoint=False).tolist()
    angles += angles[:1]

    fig, ax = plt.subplots(figsize=(7, 7), subplot_kw=dict(polar=True),
                           facecolor=BG)
    ax.set_facecolor(SURFACE)
    ax.spines["polar"].set_color(BORDER)

    for config, row in norm.iterrows():
        vals = row.tolist() + [row.iloc[0]]
        color = CONFIG_COLORS.get(config, MUTED)
        ax.plot(angles, vals, color=color, linewidth=2.5, label=config)
        ax.fill(angles, vals, color=color, alpha=0.1)

    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(labels_r, fontsize=10, color=TEXT)
    ax.set_yticks([0.25, 0.5, 0.75, 1.0])
    ax.set_yticklabels(["0.25","0.5","0.75","1.0"], fontsize=7, color=MUTED)
    ax.yaxis.grid(True, color=BORDER, linestyle="--", alpha=0.5)
    ax.xaxis.grid(True, color=BORDER, linestyle="--", alpha=0.3)
    ax.set_title("Profil comparatif des configurations\n(Quadtree, sans Hilbert — normalisé 0→1)",
                 fontsize=11, fontweight="bold", pad=20, color=TEXT)
    ax.legend(loc="upper right", bbox_to_anchor=(1.35, 1.1), fontsize=9)

    fig.tight_layout()
    savefig(fig, out / "6_radar_synthese.png")


# ══════════════════════════════════════════════════════════════════════════════
# MAIN
# ══════════════════════════════════════════════════════════════════════════════
def main():
    csv_path = sys.argv[1] if len(sys.argv) > 1 else "resultats.csv"
    df = pd.read_csv(csv_path)
    df.columns = df.columns.str.strip()
    df["Hilbert"] = df["Hilbert"].str.strip().str.upper() == "OUI"
    df["Post-Trait"] = df["Post-Trait"].fillna("None").astype(str).str.strip()
    df["Strategie"]  = df["Strategie"].str.strip()
    df["Algo"]       = df["Algo"].str.strip()

    out = Path(csv_path).parent / "graphiques"
    out.mkdir(exist_ok=True)

    print(f"\n📂 {csv_path}  →  {len(df)} lignes\n🎨 Génération dans : {out}\n")

    plot_temps(df, out)
    plot_clusters(df, out)
    plot_distance(df, out)
    plot_remplissage(df, out)
    plot_hilbert(df, out)
    plot_radar(df, out)

    print(f"\n✅ 6 graphiques générés dans : {out.resolve()}")

if __name__ == "__main__":
    main()