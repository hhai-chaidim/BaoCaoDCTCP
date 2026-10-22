#!/usr/bin/env bash
set -euo pipefail

NS3_DIR="/home/hhai/ns-allinone-3.42/ns-3.42"
PROJ_DIR="/home/hhai/BaoCao_NS3"
LOG_DIR="${PROJ_DIR}/06_KET_QUA/logs"
CSV_DIR="${PROJ_DIR}/06_KET_QUA/csv"
COMP_DIR="${PROJ_DIR}/06_KET_QUA/comparison"
FIG_DIR="${PROJ_DIR}/06_KET_QUA/figures"

mkdir -p "${LOG_DIR}" "${CSV_DIR}/S02_single_flow" "${CSV_DIR}/S03_multi_bottleneck" \
         "${CSV_DIR}/S04_queue_cdf" "${CSV_DIR}/S05_convergence" "${COMP_DIR}" "${FIG_DIR}"

cd "${NS3_DIR}"
cp "${PROJ_DIR}/04_SOURCE_CODE/modified/dctcp_experiments.cc" "${NS3_DIR}/scratch/dctcp_experiments.cc"

echo "=== [1/6] Ghi nhận môi trường & Build NS-3.42 (TC01) ==="
{
    echo "Date: $(date -Iseconds)"
    echo "Uname: $(uname -a)"
    echo "GCC: $(g++ --version | head -n 1)"
    echo "Python: $(python3 --version)"
    ./ns3 show config
} > "${LOG_DIR}/TC01_env_and_status.log" 2>&1

./ns3 build dctcp_experiments tcp-validation dctcp-example > "${LOG_DIR}/TC01_build.log" 2>&1
echo "Build exit code: $?" >> "${LOG_DIR}/TC01_build.log"

echo "=== [2/6] Chạy Smoke Test & Validation chuẩn NS-3.42: tcp-validation (TC02 / S01) ==="
./ns3 run --no-build "tcp-validation --firstTcpType=dctcp --linkRate=50Mbps --baseRtt=10ms --queueUseEcn=1 --stopTime=15s --validate=dctcp-10ms" > "${LOG_DIR}/S01_validate_dctcp_10ms.log" 2>&1
echo "Exit status dctcp-10ms: $?" >> "${LOG_DIR}/S01_validate_dctcp_10ms.log"

./ns3 run --no-build "tcp-validation --firstTcpType=dctcp --linkRate=50Mbps --baseRtt=80ms --queueUseEcn=1 --stopTime=40s --validate=dctcp-80ms" > "${LOG_DIR}/S01_validate_dctcp_80ms.log" 2>&1
echo "Exit status dctcp-80ms: $?" >> "${LOG_DIR}/S01_validate_dctcp_80ms.log"

echo "=== [3/6] Chạy Single-Flow Varying RTT & Fallback (TC06 / S02 — Paper 2 Figs 9-16) ==="
for rtt in "3us" "10ms" "50ms"; do
    echo "  -> S02 DCTCP with ECN, baseRtt=${rtt}"
    ./ns3 run --no-build "dctcp_experiments --mode=single-flow --tcpTypeId=TcpDctcp --enableSwitchEcn=true --baseRtt=${rtt} --stopTime=15s --prefix=${CSV_DIR}/S02_single_flow/S02_dctcp_ecn_${rtt}" > "${LOG_DIR}/S02_dctcp_ecn_${rtt}.log" 2>&1
done

echo "  -> S02 Fallback without ECN (DCTCP vs LinuxReno, baseRtt=10ms)"
./ns3 run --no-build "dctcp_experiments --mode=single-flow --tcpTypeId=TcpDctcp --enableSwitchEcn=false --baseRtt=10ms --stopTime=15s --prefix=${CSV_DIR}/S02_single_flow/S02_dctcp_noecn_10ms" > "${LOG_DIR}/S02_dctcp_noecn_10ms.log" 2>&1
./ns3 run --no-build "dctcp_experiments --mode=single-flow --tcpTypeId=TcpLinuxReno --enableSwitchEcn=false --baseRtt=10ms --stopTime=15s --prefix=${CSV_DIR}/S02_single_flow/S02_reno_noecn_10ms" > "${LOG_DIR}/S02_reno_noecn_10ms.log" 2>&1

echo "=== [4/6] Chạy Multi-bottleneck 40 luồng (TC03, TC04, TC05 / S03 — Paper 1 Fig 17 & Paper 2 Table 5) ==="
for run in 1 2 3; do
    echo "  -> S03a Default NS-3.42 (seed=${run})"
    ./ns3 run --no-build "dctcp_experiments --mode=multi-bottleneck --tcpTypeId=TcpDctcp --enableSwitchEcn=true --instantMarking=false --rngRun=${run} --prefix=${CSV_DIR}/S03_multi_bottleneck/S03a_default_seed${run}" > "${LOG_DIR}/S03a_default_seed${run}.log" 2>&1
done

echo "  -> S03a Repeat seed=1 (TC05 determinism check)"
./ns3 run --no-build "dctcp_experiments --mode=multi-bottleneck --tcpTypeId=TcpDctcp --enableSwitchEcn=true --instantMarking=false --rngRun=1 --prefix=${CSV_DIR}/S03_multi_bottleneck/S03a_default_seed1_repeat" > "${LOG_DIR}/S03a_default_seed1_repeat.log" 2>&1

echo "  -> S03b Instantaneous marking MinTh=MaxTh=K (K1G=20, K10G=65)"
./ns3 run --no-build "dctcp_experiments --mode=multi-bottleneck --tcpTypeId=TcpDctcp --enableSwitchEcn=true --instantMarking=true --k1G=20 --k10G=65 --rngRun=1 --prefix=${CSV_DIR}/S03_multi_bottleneck/S03b_instantK_seed1" > "${LOG_DIR}/S03b_instantK_seed1.log" 2>&1

echo "  -> S03c Baseline TCP without ECN"
./ns3 run --no-build "dctcp_experiments --mode=multi-bottleneck --tcpTypeId=TcpLinuxReno --enableSwitchEcn=false --instantMarking=false --rngRun=1 --prefix=${CSV_DIR}/S03_multi_bottleneck/S03c_tcp_noecn_seed1" > "${LOG_DIR}/S03c_tcp_noecn_seed1.log" 2>&1

echo "=== [5/6] Chạy Queue Length Sawtooth & CDF 1Gbps (TC06 / S04 — Paper 1 Fig 1 & Fig 13) ==="
for n in 2 20; do
    echo "  -> S04 DCTCP N=${n} flows"
    ./ns3 run --no-build "dctcp_experiments --mode=queue-cdf --tcpTypeId=TcpDctcp --enableSwitchEcn=true --k1G=20 --nFlows=${n} --stopTime=5s --prefix=${CSV_DIR}/S04_queue_cdf/S04_dctcp_${n}flows" > "${LOG_DIR}/S04_dctcp_${n}flows.log" 2>&1

    echo "  -> S04 TCP (no ECN) N=${n} flows"
    ./ns3 run --no-build "dctcp_experiments --mode=queue-cdf --tcpTypeId=TcpLinuxReno --enableSwitchEcn=false --k1G=20 --nFlows=${n} --stopTime=5s --prefix=${CSV_DIR}/S04_queue_cdf/S04_tcp_${n}flows" > "${LOG_DIR}/S04_tcp_${n}flows.log" 2>&1
done

echo "=== [6/6] Chạy Convergence 5 luồng & Phân tích/Vẽ biểu đồ (TC06 / S05 — Paper 1 Fig 16) ==="
echo "  -> S05 DCTCP 5 flows convergence"
./ns3 run --no-build "dctcp_experiments --mode=convergence --tcpTypeId=TcpDctcp --enableSwitchEcn=true --stepTime=3s --prefix=${CSV_DIR}/S05_convergence/S05_dctcp_5flows" > "${LOG_DIR}/S05_dctcp_5flows.log" 2>&1

echo "  -> S05 TCP (no ECN) 5 flows convergence"
./ns3 run --no-build "dctcp_experiments --mode=convergence --tcpTypeId=TcpLinuxReno --enableSwitchEcn=false --stepTime=3s --prefix=${CSV_DIR}/S05_convergence/S05_tcp_5flows" > "${LOG_DIR}/S05_tcp_5flows.log" 2>&1

python3 "${PROJ_DIR}/04_SOURCE_CODE/scripts/analyze_and_plot.py" > "${LOG_DIR}/analyze_and_plot.log" 2>&1

echo "=== HOÀN TẤT TOÀN BỘ MÔ PHỎNG VÀ PHÂN TÍCH ==="
