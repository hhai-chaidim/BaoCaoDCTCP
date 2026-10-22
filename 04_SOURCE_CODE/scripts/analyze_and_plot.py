#!/usr/bin/env python3
"""
Phân tích dữ liệu mô phỏng thực tế trên NS-3.42, kiểm tra TC01–TC06,
tạo bảng đối chiếu CSV/Markdown và vẽ biểu đồ so sánh với Bài báo 1 & Bài báo 2.
"""

import csv
import math
import os
import re
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

PROJ_DIR = Path("/home/hhai/BaoCao_NS3")
CSV_DIR = PROJ_DIR / "06_KET_QUA" / "csv"
COMP_DIR = PROJ_DIR / "06_KET_QUA" / "comparison"
FIG_DIR = PROJ_DIR / "06_KET_QUA" / "figures"

COMP_DIR.mkdir(parents=True, exist_ok=True)
FIG_DIR.mkdir(parents=True, exist_ok=True)


def parse_fairness_file(filepath: Path):
    """Đọc tệp *-fairness.dat từ kịch bản S03 (Multi-bottleneck)."""
    res = {
        "S1_mbps": 0.0,
        "S1_fairness": 0.0,
        "S2_mbps": 0.0,
        "S2_fairness": 0.0,
        "S3_mbps": 0.0,
        "S3_fairness": 0.0,
        "T1_gbps": 0.0,
        "R1_gbps": 0.0,
        "overall_fairness": 0.0,
    }
    if not filepath.exists():
        return res
    text = filepath.read_text(encoding="utf-8")
    for line in text.splitlines():
        m = re.search(r"Average throughput for (S1-R1|S2-R2|S3-R1) flows:\s*([\d.]+)\s*Mbps;\s*fairness:\s*([\d.]+)", line)
        if m:
            grp, mbps, fair = m.group(1), float(m.group(2)), float(m.group(3))
            prefix = grp.split("-")[0]
            res[f"{prefix}_mbps"] = mbps
            res[f"{prefix}_fairness"] = fair
        m_t1 = re.search(r"Aggregate user-level throughput for flows through T1:\s*([\d.]+)\s*Gbps", line)
        if m_t1:
            res["T1_gbps"] = float(m_t1.group(1))
        m_r1 = re.search(r"Aggregate user-level throughput for flows to R1:\s*([\d.]+)\s*Gbps", line)
        if m_r1:
            res["R1_gbps"] = float(m_r1.group(1))
        m_ov = re.search(r"Normalized overall Jain fairness index \(40 flows\):\s*([\d.]+)", line)
        if m_ov:
            res["overall_fairness"] = float(m_ov.group(1))
    return res


def parse_queue_dat(filepath: Path):
    """Đọc tệp *-t1-length.dat hoặc *-t2-length.dat."""
    pkts, delays = [], []
    if not filepath.exists():
        return {"mean_pkts": 0.0, "p95_pkts": 0.0, "max_pkts": 0, "mean_us": 0.0, "p95_us": 0.0}
    for line in filepath.read_text(encoding="utf-8").splitlines():
        if line.startswith("#") or not line.strip():
            continue
        parts = line.split()
        if len(parts) >= 3:
            pkts.append(float(parts[1]))
            delays.append(float(parts[2]))
    if not pkts:
        return {"mean_pkts": 0.0, "p95_pkts": 0.0, "max_pkts": 0, "mean_us": 0.0, "p95_us": 0.0}
    arr_p = np.array(pkts)
    arr_d = np.array(delays)
    return {
        "mean_pkts": float(np.mean(arr_p)),
        "p95_pkts": float(np.percentile(arr_p, 95)),
        "max_pkts": int(np.max(arr_p)),
        "mean_us": float(np.mean(arr_d)),
        "p95_us": float(np.percentile(arr_d, 95)),
    }


def analyze_s03_multi_bottleneck():
    """Phân tích kịch bản S03 (Multi-bottleneck 40 luồng) và vẽ biểu đồ so sánh Bảng 5 Bài báo 2 & Hình 17 Bài báo 1."""
    s03_dir = CSV_DIR / "S03_multi_bottleneck"
    runs = {
        "S03a_seed1": s03_dir / "S03a_default_seed1",
        "S03a_seed2": s03_dir / "S03a_default_seed2",
        "S03a_seed3": s03_dir / "S03a_default_seed3",
        "S03a_seed1_repeat": s03_dir / "S03a_default_seed1_repeat",
        "S03b_instantK": s03_dir / "S03b_instantK_seed1",
        "S03c_tcp_noecn": s03_dir / "S03c_tcp_noecn_seed1",
    }

    summary_rows = []
    for name, prefix in runs.items():
        f_stats = parse_fairness_file(Path(f"{prefix}-fairness.dat"))
        t1_q = parse_queue_dat(Path(f"{prefix}-t1-length.dat"))
        t2_q = parse_queue_dat(Path(f"{prefix}-t2-length.dat"))
        row = {
            "run_id": name,
            **f_stats,
            "T1_mean_pkts": round(t1_q["mean_pkts"], 2),
            "T1_p95_pkts": round(t1_q["p95_pkts"], 2),
            "T1_mean_us": round(t1_q["mean_us"], 2),
            "T2_mean_pkts": round(t2_q["mean_pkts"], 2),
            "T2_p95_pkts": round(t2_q["p95_pkts"], 2),
            "T2_mean_us": round(t2_q["mean_us"], 2),
        }
        summary_rows.append(row)

    # Lưu CSV tổng hợp S03
    out_csv = COMP_DIR / "S03_multi_bottleneck_comparison.csv"
    if summary_rows:
        with open(out_csv, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=list(summary_rows[0].keys()))
            writer.writeheader()
            writer.writerows(summary_rows)

    # Vẽ biểu đồ cột đối chiếu S1, S2, S3 giữa Bài báo 1, Bài báo 2 và NS-3.42
    by_id = {r["run_id"]: r for r in summary_rows}
    s03a = by_id.get("S03a_seed1", {})
    s03b = by_id.get("S03b_instantK", {})
    s03c = by_id.get("S03c_tcp_noecn", {})

    labels = ["S1 flows\n(10 flows, 2 bottlenecks)", "S2 flows\n(20 flows, 10G bottleneck)", "S3 flows\n(10 flows, 1G bottleneck)"]
    paper1_vals = [46.0, 475.0, 54.0]
    paper2_vals = [23.0, 471.0, 74.0]
    ns3_s03a_vals = [s03a.get("S1_mbps", 0), s03a.get("S2_mbps", 0), s03a.get("S3_mbps", 0)]
    ns3_s03b_vals = [s03b.get("S1_mbps", 0), s03b.get("S2_mbps", 0), s03b.get("S3_mbps", 0)]
    ns3_s03c_vals = [s03c.get("S1_mbps", 0), s03c.get("S2_mbps", 0), s03c.get("S3_mbps", 0)]

    x = np.arange(len(labels))
    width = 0.16

    fig, ax = plt.subplots(figsize=(10, 5.5), dpi=150)
    ax.bar(x - 2 * width, paper1_vals, width, label="Paper 1 (SIGCOMM'10 Testbed)", color="#1f77b4")
    ax.bar(x - width, paper2_vals, width, label="Paper 2 (WNS3'20 Published)", color="#ff7f0e")
    ax.bar(x, ns3_s03a_vals, width, label="NS-3.42 S03a (Default 20/60 & 50/150)", color="#2ca02c")
    ax.bar(x + width, ns3_s03b_vals, width, label="NS-3.42 S03b (Instant K=20 & K=65)", color="#d62728")
    ax.bar(x + 2 * width, ns3_s03c_vals, width, label="NS-3.42 S03c (TCP No-ECN Baseline)", color="#7f7f7f")

    ax.set_ylabel("Average Per-Flow Throughput (Mbps)")
    ax.set_title("Multi-Bottleneck 40-Flow Topology (Fig. 17 [1] & Table 5 [2]): Throughput Comparison")
    ax.set_xticks(x)
    ax.set_xticklabels(labels)
    ax.legend(loc="upper right", fontsize=8.5)
    ax.grid(axis="y", linestyle="--", alpha=0.5)

    for i, v in enumerate(ns3_s03a_vals):
        ax.text(x[i], v + 8, f"{v:.1f}", ha="center", va="bottom", fontsize=7.5, fontweight="bold")
    for i, v in enumerate(ns3_s03b_vals):
        ax.text(x[i] + width, v + 8, f"{v:.1f}", ha="center", va="bottom", fontsize=7.5)

    fig.tight_layout()
    fig.savefig(FIG_DIR / "fig1_S03_multi_bottleneck_throughput.png")
    plt.close(fig)

    return by_id


def analyze_s02_single_flow():
    """Phân tích và vẽ biểu đồ kịch bản S02 (Single-flow varying RTT & Fallback — Hình 9-16 Bài báo 2)."""
    s02_dir = CSV_DIR / "S02_single_flow"

    # 1. Biểu đồ cwnd & queue delay cho RTT = 3us, 10ms, 50ms
    fig, axes = plt.subplots(3, 2, figsize=(11, 8.5), dpi=150)
    rtts = [("3us", "3 us RTT (Fig. 9 [2])"), ("10ms", "10 ms RTT (Fig. 10 [2])"), ("50ms", "50 ms RTT (Fig. 11 [2])")]

    s02_stats = []
    for row_idx, (rtt_key, title_label) in enumerate(rtts):
        cwnd_file = s02_dir / f"S02_dctcp_ecn_{rtt_key}-cwnd.csv"
        q_file = s02_dir / f"S02_dctcp_ecn_{rtt_key}-queue.csv"
        tput_file = s02_dir / f"S02_dctcp_ecn_{rtt_key}-throughput.csv"

        t_c, v_c = [], []
        if cwnd_file.exists():
            with open(cwnd_file, "r", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                for r in reader:
                    t_c.append(float(r["time_s"]))
                    v_c.append(float(r["cwnd_segments"]))

        t_q, v_q_ms, v_q_pkts = [], [], []
        if q_file.exists():
            with open(q_file, "r", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                for r in reader:
                    t_q.append(float(r["time_s"]))
                    v_q_pkts.append(float(r["queue_pkts"]))
                    v_q_ms.append(float(r["queue_delay_ms"]))

        v_tput = []
        if tput_file.exists():
            with open(tput_file, "r", encoding="utf-8") as f:
                reader = csv.DictReader(f)
                for r in reader:
                    if float(r["time_s"]) >= 5.0:
                        v_tput.append(float(r["throughput_mbps"]))

        steady_cwnd = [v_c[i] for i in range(len(t_c)) if t_c[i] >= 5.0]
        steady_q_ms = [v_q_ms[i] for i in range(len(t_q)) if t_q[i] >= 5.0]
        steady_q_pkts = [v_q_pkts[i] for i in range(len(t_q)) if t_q[i] >= 5.0]

        s02_stats.append({
            "case": f"DCTCP_ECN_{rtt_key}",
            "steady_cwnd_mean_seg": round(float(np.mean(steady_cwnd)), 2) if steady_cwnd else 0.0,
            "steady_cwnd_min_seg": round(float(np.min(steady_cwnd)), 2) if steady_cwnd else 0.0,
            "steady_cwnd_max_seg": round(float(np.max(steady_cwnd)), 2) if steady_cwnd else 0.0,
            "steady_q_delay_mean_ms": round(float(np.mean(steady_q_ms)), 2) if steady_q_ms else 0.0,
            "steady_q_pkts_mean": round(float(np.mean(steady_q_pkts)), 2) if steady_q_pkts else 0.0,
            "steady_tput_mean_mbps": round(float(np.mean(v_tput)), 2) if v_tput else 0.0,
        })

        axes[row_idx, 0].plot(t_c, v_c, color="#1f77b4", linewidth=1.0)
        axes[row_idx, 0].set_title(f"{title_label} — Congestion Window")
        axes[row_idx, 0].set_xlabel("Time (s)")
        axes[row_idx, 0].set_ylabel("cwnd (segments)")
        axes[row_idx, 0].grid(True, linestyle="--", alpha=0.5)

        axes[row_idx, 1].plot(t_q, v_q_ms, color="#2ca02c", linewidth=1.0)
        axes[row_idx, 1].set_title(f"{title_label} — Queue Occupancy")
        axes[row_idx, 1].set_xlabel("Time (s)")
        axes[row_idx, 1].set_ylabel("Queue Delay (ms)")
        axes[row_idx, 1].grid(True, linestyle="--", alpha=0.5)

    fig.tight_layout()
    fig.savefig(FIG_DIR / "fig2_S02_single_flow_varying_rtt.png")
    plt.close(fig)

    # 2. Biểu đồ Fallback to NewReno/LinuxReno khi tắt ECN (Hình 15 & 16 Bài báo 2)
    fig, axes = plt.subplots(1, 2, figsize=(10.5, 4.2), dpi=150)
    for idx, (key, title_str, col) in enumerate([
        ("S02_reno_noecn_10ms", "LinuxReno without ECN (Fig. 15 [2])", "#ff7f0e"),
        ("S02_dctcp_noecn_10ms", "DCTCP without ECN (Fig. 16 [2])", "#1f77b4"),
    ]):
        cwnd_file = s02_dir / f"{key}-cwnd.csv"
        q_file = s02_dir / f"{key}-queue.csv"
        tput_file = s02_dir / f"{key}-throughput.csv"

        t_c, v_c = [], []
        if cwnd_file.exists():
            with open(cwnd_file, "r", encoding="utf-8") as f:
                for r in csv.DictReader(f):
                    t_c.append(float(r["time_s"]))
                    v_c.append(float(r["cwnd_segments"]))
        # TracedValue chỉ ghi khi cwnd thay đổi; giữ nguyên giá trị cuối đến hết 15s nếu không đổi thêm
        if t_c and t_c[-1] < 15.0:
            t_c.append(15.0)
            v_c.append(v_c[-1])

        t_q, v_q_ms, v_q_pkts = [], [], []
        if q_file.exists():
            with open(q_file, "r", encoding="utf-8") as f:
                for r in csv.DictReader(f):
                    t_q.append(float(r["time_s"]))
                    v_q_pkts.append(float(r["queue_pkts"]))
                    v_q_ms.append(float(r["queue_delay_ms"]))

        v_tput = []
        if tput_file.exists():
            with open(tput_file, "r", encoding="utf-8") as f:
                for r in csv.DictReader(f):
                    if float(r["time_s"]) >= 5.0:
                        v_tput.append(float(r["throughput_mbps"]))

        axes[idx].step(t_c, v_c, where="post", color=col, linewidth=1.1)
        axes[idx].set_title(title_str)
        axes[idx].set_xlabel("Time (s)")
        axes[idx].set_ylabel("cwnd (segments)")
        axes[idx].set_xlim(0, 15.0)
        axes[idx].grid(True, linestyle="--", alpha=0.5)

        steady_cwnd = [v_c[i] for i in range(len(t_c)) if t_c[i] >= 5.0]
        steady_q_ms = [v_q_ms[i] for i in range(len(t_q)) if t_q[i] >= 5.0]
        steady_q_pkts = [v_q_pkts[i] for i in range(len(t_q)) if t_q[i] >= 5.0]
        s02_stats.append({
            "case": key,
            "steady_cwnd_mean_seg": round(float(np.mean(steady_cwnd)), 2) if steady_cwnd else 0.0,
            "steady_cwnd_min_seg": round(float(np.min(steady_cwnd)), 2) if steady_cwnd else 0.0,
            "steady_cwnd_max_seg": round(float(np.max(steady_cwnd)), 2) if steady_cwnd else 0.0,
            "steady_q_delay_mean_ms": round(float(np.mean(steady_q_ms)), 2) if steady_q_ms else 0.0,
            "steady_q_pkts_mean": round(float(np.mean(steady_q_pkts)), 2) if steady_q_pkts else 0.0,
            "steady_tput_mean_mbps": round(float(np.mean(v_tput)), 2) if v_tput else 0.0,
        })

    fig.tight_layout()
    fig.savefig(FIG_DIR / "fig3_S02_fallback_no_ecn.png")
    plt.close(fig)

    out_csv = COMP_DIR / "S02_single_flow_summary.csv"
    if s02_stats:
        with open(out_csv, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=list(s02_stats[0].keys()))
            writer.writeheader()
            writer.writerows(s02_stats)

    return s02_stats


def analyze_s04_queue_cdf():
    """Phân tích và vẽ biểu đồ kịch bản S04 (Queue Length Sawtooth Hình 1 & CDF Hình 13 Bài báo 1)."""
    s04_dir = CSV_DIR / "S04_queue_cdf"
    cases = [
        ("S04_dctcp_2flows", "DCTCP (2 flows)", "#1f77b4", "-"),
        ("S04_dctcp_20flows", "DCTCP (20 flows)", "#2ca02c", "--"),
        ("S04_tcp_2flows", "TCP DropTail (2 flows)", "#d62728", "-."),
        ("S04_tcp_20flows", "TCP DropTail (20 flows)", "#9467bd", ":"),
    ]

    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.6), dpi=150)
    s04_rows = []

    for prefix_name, label, color, ls in cases:
        q_csv = s04_dir / f"{prefix_name}-queue.csv"
        sum_txt = s04_dir / f"{prefix_name}-summary.txt"
        t_list, pkts_list, kb_list = [], [], []
        if q_csv.exists():
            with open(q_csv, "r", encoding="utf-8") as f:
                for r in csv.DictReader(f):
                    t_list.append(float(r["time_s"]))
                    pkts_list.append(float(r["queue_pkts"]))
                    kb_list.append(float(r["queue_kb"]))

        tput_gbps = 0.0
        if sum_txt.exists():
            for line in sum_txt.read_text(encoding="utf-8").splitlines():
                if line.startswith("aggregateThroughputGbps="):
                    tput_gbps = float(line.split("=")[1])

        if pkts_list:
            arr_p = np.array(pkts_list)
            arr_kb = np.array(kb_list)
            s04_rows.append({
                "case": prefix_name,
                "throughput_gbps": round(tput_gbps, 4),
                "mean_pkts": round(float(np.mean(arr_p)), 2),
                "median_pkts": round(float(np.median(arr_p)), 2),
                "p90_pkts": round(float(np.percentile(arr_p, 90)), 2),
                "p99_pkts": round(float(np.percentile(arr_p, 99)), 2),
                "max_pkts": int(np.max(arr_p)),
                "mean_kb": round(float(np.mean(arr_kb)), 2),
                "max_kb": round(float(np.max(arr_kb)), 2),
            })

            # Plot Time Series (2 flows only, matching Fig. 1 [1])
            if "2flows" in prefix_name:
                mask = [i for i, t in enumerate(t_list) if 1.0 <= t <= 4.0]
                t_ms = [(t_list[i] - 1.0) * 1000.0 for i in mask]
                kb_sub = [kb_list[i] for i in mask]
                axes[0].plot(t_ms, kb_sub, label=label, color=color, linestyle=ls, linewidth=1.2)

            # Plot CDF of queue length in packets (matching Fig. 13 [1])
            sorted_p = np.sort(arr_p)
            cdf = np.arange(1, len(sorted_p) + 1) / len(sorted_p)
            axes[1].plot(sorted_p, cdf, label=label, color=color, linestyle=ls, linewidth=1.6)

    axes[0].set_title("Queue Length Time Series — 2 Flows (Fig. 1 [1])")
    axes[0].set_xlabel("Time (ms)")
    axes[0].set_ylabel("Instantaneous Queue Length (KB)")
    axes[0].legend(loc="upper right")
    axes[0].grid(True, linestyle="--", alpha=0.5)

    axes[1].set_title("Queue Length CDF at 1 Gbps (Fig. 13 [1])")
    axes[1].set_xlabel("Queue Length (packets)")
    axes[1].set_ylabel("Cumulative Fraction (CDF)")
    axes[1].legend(loc="lower right")
    axes[1].grid(True, linestyle="--", alpha=0.5)

    fig.tight_layout()
    fig.savefig(FIG_DIR / "fig4_S04_queue_length_sawtooth_and_cdf.png")
    plt.close(fig)

    out_csv = COMP_DIR / "S04_queue_cdf_summary.csv"
    if s04_rows:
        with open(out_csv, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=list(s04_rows[0].keys()))
            writer.writeheader()
            writer.writerows(s04_rows)

    return s04_rows


def analyze_s05_convergence():
    """Phân tích và vẽ biểu đồ kịch bản S05 (Convergence 5 luồng — Hình 16 Bài báo 1)."""
    s05_dir = CSV_DIR / "S05_convergence"
    fig, axes = plt.subplots(1, 2, figsize=(11.5, 4.5), dpi=150)

    for idx, (prefix_name, title_str) in enumerate([
        ("S05_dctcp_5flows", "(a) DCTCP Convergence (Fig. 16a [1])"),
        ("S05_tcp_5flows", "(b) TCP Convergence (Fig. 16b [1])"),
    ]):
        csv_file = s05_dir / f"{prefix_name}-convergence.csv"
        if not csv_file.exists():
            continue
        t_vals = []
        flows = [[] for _ in range(5)]
        with open(csv_file, "r", encoding="utf-8") as f:
            for r in csv.DictReader(f):
                t_vals.append(float(r["time_s"]))
                for i in range(5):
                    flows[i].append(float(r[f"flow{i+1}_gbps"]))
        for i in range(5):
            axes[idx].plot(t_vals, flows[i], label=f"Flow {i+1}", linewidth=1.2)
        axes[idx].set_title(title_str)
        axes[idx].set_xlabel("Time (seconds)")
        axes[idx].set_ylabel("Throughput (Gbps)")
        axes[idx].set_ylim(0, 1.05)
        axes[idx].legend(loc="upper right", fontsize=8)
        axes[idx].grid(True, linestyle="--", alpha=0.5)

    fig.tight_layout()
    fig.savefig(FIG_DIR / "fig5_S05_convergence_5flows.png")
    plt.close(fig)


if __name__ == "__main__":
    print("Analyzing S03 Multi-bottleneck...")
    s03_res = analyze_s03_multi_bottleneck()
    print("Analyzing S02 Single-flow & Fallback...")
    s02_res = analyze_s02_single_flow()
    print("Analyzing S04 Queue Length & CDF...")
    s04_res = analyze_s04_queue_cdf()
    print("Analyzing S05 Convergence...")
    analyze_s05_convergence()
    print("All analyses and figures generated successfully in 06_KET_QUA/.")
