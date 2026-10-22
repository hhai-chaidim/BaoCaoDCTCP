# Nghiên cứu, Hiện thực hóa và Kiểm chứng Giao thức Data Center TCP (DCTCP) trên NS-3.42

Dự án bài tập lớn môn **Mô phỏng mạng với NS-3**, tập trung phân tích lý thuyết, khảo sát kiến trúc hiện thực hóa, thiết kế kịch bản mô phỏng, kiểm thử tự động và đối chiếu thực nghiệm giao thức **Data Center TCP (DCTCP)** trên bộ mô phỏng **NS-3.42** dựa trên hai công trình nghiên cứu nền tảng:

1. **Bài báo 1 (Lý thuyết & Thiết kế gốc)**: Mohammad Alizadeh, Albert Greenberg, David A. Maltz, Jitendra Padhye, Parveen Patel, Balaji Prabhakar, Sudipta Sengupta, Murari Sridharan, *"Data Center TCP (DCTCP)"*, ACM SIGCOMM 2010, pp. 63–74.
2. **Bài báo 2 (Thực nghiệm & Mô hình NS-3)**: Vivek Jain, Thomas R. Henderson, Shravya K. S., Mohit P. Tahiliani, *"Data Center TCP in ns-3: Implementation, Validation and Evaluation"*, ACM WNS3 2020, pp. 65–72.

---

## 1. Cấu trúc Thư mục Dự án

```text
BaoCao_NS3/
├── 01_TAI_LIEU_GOC/
│   └── YEU_CAU_MON_HOC/
│       └── Yêu cầu.pdf                          # Đề bài và 6 yêu cầu của bài tập lớn
├── 03_DAC_TA_CHUNG/
│   ├── model_spec.md                            # Đặc tả toán học và ánh xạ mô hình DCTCP trên NS-3.42
│   ├── scenario_spec.md                         # Đặc tả chi tiết 5 kịch bản thực nghiệm (S01 – S05)
│   └── test_plan.md                             # Kế hoạch kiểm thử và kết quả nghiệm thu (TC01 – TC06)
├── 04_SOURCE_CODE/
│   ├── original/                                # Mã nguồn gốc DCTCP và kịch bản mẫu trích từ NS-3.42
│   ├── modified/
│   │   └── dctcp_experiments.cc                 # Chương trình C++ hợp nhất các kịch bản thực nghiệm S02 – S05
│   ├── scripts/
│   │   ├── run_all_experiments.sh               # Script tự động biên dịch và chạy toàn bộ kịch bản S01 – S05
│   │   └── analyze_and_plot.py                  # Script xử lý dữ liệu CSV và vẽ 5 biểu đồ kết quả
│   └── README.md                                # Hướng dẫn biên dịch và thực thi mã nguồn
├── 06_KET_QUA/
│   ├── csv/                                     # Dữ liệu kết quả thực nghiệm (.csv, .dat, .txt) của S02 – S05
│   ├── comparison/                              # Bảng tổng hợp CSV và báo cáo đối chiếu định lượng
│   ├── figures/                                 # 5 biểu đồ kết quả thực nghiệm (PNG, 300 DPI)
│   └── README.md                                # Mô tả cấu trúc dữ liệu kết quả
└── 07_BAO_CAO/
    ├── final/
    │   └── BAO_CAO_BAI_TAP_LON_DCTCP_NS3_42.md  # Báo cáo tổng kết chính thức trả lời 6 yêu cầu môn học
    └── README.md                                # Danh mục tài liệu báo cáo
```

---

## 2. Tóm tắt Kết quả Chính

1. **Kiểm chứng chuẩn NS-3 (`S01`)**: Vượt qua 100% các bài kiểm tra tự động của `tcp-validation` với `--validate=dctcp-10ms` và `--validate=dctcp-80ms` (`exit code 0`, thông lượng đạt `48.2 Mbps` trên liên kết cổ chai `50 Mbps`).
2. **Kịch bản Đa cổ chai 40 luồng (`S03` — Hình 17 Bài báo 1 & Bảng 5 Bài báo 2)**:
   - Cấu hình mặc định của NS-3.42 (`S03a`, trung bình 3 seed): đạt **S1 = `23.65 Mbps`** ($J=0.99$), **S2 = `470.04 Mbps`** ($J=1.00$), **S3 = `72.75 Mbps`** ($J=0.998$), khớp sát công bố trong Bảng 5 Bài báo 2 (`23 / 471 / 74 Mbps`, $J=0.99$) và khớp thông lượng nhóm S2 của Bài báo 1 (`475 Mbps`).
   - Cấu hình đánh dấu tức thời `MinTh = MaxTh = K` (`S03b` với $K_{1G}=20, K_{10G}=65$): giảm độ dài hàng đợi trung bình tại chuyển mạch 10Gbps (T1) **55%** (`176.0` $\to$ `79.5` gói) và tại chuyển mạch 1Gbps (T2) **66%** (`75.9` $\to$ `25.8` gói) trong khi vẫn duy trì tổng thông lượng `9.637 Gbps` tại T1.
3. **Kịch bản Hàng đợi Răng cưa & CDF tại 1Gbps (`S04` — Hình 1 & Hình 13 Bài báo 1)**:
   - Với $N=2$ luồng và $K=20$ gói, hàng đợi DCTCP đạt trung bình **`21.54` gói (`32.31 KB`)** và trung vị **`22.0` gói**, khớp chính xác với công thức lý thuyết $Q_{\max} \approx K + N = 22$ gói (Phương trình 10 Bài báo 1).
   - Giảm **92.8%** mức chiếm dụng bộ đệm so với TCP DropTail (`32.31 KB` so với `448.22 KB` ở 2 luồng; `40.67 KB` so với `554.36 KB` ở 20 luồng) trong khi đạt thông lượng cao hơn (`0.9640 Gbps` so với `0.9491 Gbps`).
4. **Kịch bản Hội tụ 5 luồng (`S05` — Hình 16 Bài báo 1) & Đơn luồng biến thiên RTT (`S02` — Hình 9–16 Bài báo 2)**:
   - Chứng minh khả năng hội tụ nhanh và phân chia công bằng $\frac{1}{k}$ băng thông của DCTCP so với dao động mạnh của TCP truyền thống, đồng thời làm rõ hành vi của `TcpDctcp::GetSsThresh()` khi bộ định tuyến không bật ECN trên NS-3.42.

---

## 3. Hướng dẫn Biên dịch và Chạy Tái lập

Yêu cầu hệ thống: Hệ điều hành Linux (Ubuntu), đã cài đặt **NS-3.42** tại `/home/hhai/ns-allinone-3.42/ns-3.42` và Python 3 (`numpy`, `matplotlib`).

```bash
# 1. Chạy toàn bộ quá trình biên dịch, kiểm chứng S01 và các kịch bản thực nghiệm S02 -> S05
bash 04_SOURCE_CODE/scripts/run_all_experiments.sh

# 2. Phân tích dữ liệu CSV, xuất bảng đối chiếu và vẽ lại 5 biểu đồ trong 06_KET_QUA/figures/
python3 04_SOURCE_CODE/scripts/analyze_and_plot.py
```

Xem toàn bộ nội dung báo cáo chi tiết tại: **[`07_BAO_CAO/final/BAO_CAO_BAI_TAP_LON_DCTCP_NS3_42.md`](07_BAO_CAO/final/BAO_CAO_BAI_TAP_LON_DCTCP_NS3_42.md)**.
