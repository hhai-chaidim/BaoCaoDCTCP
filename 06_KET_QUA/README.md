# Kết quả Thực nghiệm (`06_KET_QUA/`)

Thư mục này lưu trữ toàn bộ dữ liệu đầu ra từ các kịch bản mô phỏng trên NS-3.42, bảng tổng hợp đối chiếu và biểu đồ minh họa:

1. **`csv/` — Dữ liệu thực nghiệm gốc của từng kịch bản**:
   - `S02_single_flow/`: Dữ liệu `cwnd`, `alpha`, `queue` và `throughput` theo thời gian cho kịch bản đơn luồng `50 Mbps` với $\text{RTT} \in \{3\mu\text{s}, 10\text{ms}, 50\text{ms}\}$ và trường hợp tắt ECN.
   - `S03_multi_bottleneck/`: 6 tệp `.dat` đầu ra cho mỗi lượt chạy của kịch bản đa cổ chai 40 luồng (`S03a_default_seed1..3`, `S03a_default_seed1_repeat`, `S03b_instantK_seed1`, `S03c_tcp_noecn_seed1`).
   - `S04_queue_cdf/`: Chuỗi thời gian độ dài hàng đợi (`-queue.csv`) và thông lượng tổng hợp (`-summary.txt`) của DCTCP và TCP DropTail với $N=2$ và $N=20$ luồng.
   - `S05_convergence/`: Chuỗi thời gian thông lượng của 5 luồng khởi động/kết thúc lệch pha (`S05_dctcp_5flows-convergence.csv`, `S05_tcp_5flows-convergence.csv`).
2. **`comparison/` — Bảng tổng hợp và Báo cáo đối chiếu định lượng**:
   - `S03_multi_bottleneck_comparison.csv`: Bảng tổng hợp thông lượng, chỉ số công bằng Jain và độ dài hàng đợi T1/T2 của kịch bản `S03`.
   - `S04_queue_cdf_summary.csv`: Bảng thống kê các phân vị (Mean, Median, P90, P99, Max) của hàng đợi kịch bản `S04`.
   - `S02_single_flow_summary.csv`: Bảng thống kê `cwnd`, trễ hàng đợi và thông lượng xác lập của kịch bản `S02`.
   - `bao_cao_doi_chieu_ket_qua_T06.md`: Tài liệu phân tích và đối chiếu chi tiết các bảng kết quả với hai bài báo gốc.
3. **`figures/` — Biểu đồ kết quả thực nghiệm (300 DPI)**:
   - `fig1_S03_multi_bottleneck_throughput.png`
   - `fig2_S02_single_flow_varying_rtt.png`
   - `fig3_S02_fallback_no_ecn.png`
   - `fig4_S04_queue_length_sawtooth_and_cdf.png`
   - `fig5_S05_convergence_5flows.png`
