# Mã nguồn và Hướng dẫn Tái lập Thực nghiệm trên NS-3.42

## 1. Môi trường Hệ thống
- **Hệ điều hành**: Ubuntu Linux (`x86_64`)
- **Phiên bản NS-3**: `ns-3.42` (cài đặt tại `/home/hhai/ns-allinone-3.42/ns-3.42`)
- **Trình biên dịch**: GCC `g++` (CMake/Ninja thông qua `./ns3`)
- **Công cụ phân tích & vẽ đồ thị**: Python 3 (`numpy`, `matplotlib`)

## 2. Cấu trúc Thư mục `04_SOURCE_CODE/`
- `original/`: Bản sao nguyên gốc các tệp mã nguồn DCTCP trong NS-3.42 để đối chiếu:
  - `tcp-dctcp.h`, `tcp-dctcp.cc` (`src/internet/model/`) — Mô hình giao thức `TcpDctcp`.
  - `tcp-dctcp-test.cc`, `tcp-ecn-test.cc` (`src/internet/test/`) — Bộ kiểm thử đơn vị cho DCTCP và ECN.
  - `dctcp-example.cc`, `tcp-validation.cc` (`examples/tcp/`) — Hai kịch bản mẫu của nhóm tác giả Bài báo 2.
- `modified/`:
  - `dctcp_experiments.cc`: Chương trình mô phỏng hợp nhất cho toàn bộ các kịch bản thực nghiệm:
    - `--mode=single-flow` (**S02**): Dumbbell 1 luồng `50 Mbps` với RTT biến thiên (`3us`, `10ms`, `50ms`) và kiểm tra hành vi khi tắt ECN trên router.
    - `--mode=multi-bottleneck` (**S03**): Mạng đa cổ chai 40 luồng (`S1: 10, S2: 20, S3: 10`) hỗ trợ tùy chọn đánh dấu tức thời `--instantMarking=true/false` và bật/tắt ECN.
    - `--mode=queue-cdf` (**S04**): Đo chuỗi thời gian răng cưa và phân phối tích lũy CDF của độ dài hàng đợi tại `1 Gbps` với $N \in \{2, 20\}$ luồng.
    - `--mode=convergence` (**S05**): Kiểm tra tính hội tụ động của 5 luồng khởi động và kết thúc lệch pha trên cổ chai `1 Gbps`.
- `scripts/`:
  - `run_all_experiments.sh`: Script tự động sao chép `dctcp_experiments.cc` vào `ns-3.42/scratch/`, biên dịch và chạy toàn bộ các kịch bản từ `S01` đến `S05`.
  - `analyze_and_plot.py`: Script Python xử lý dữ liệu trong `06_KET_QUA/csv/`, tạo các bảng tổng hợp trong `06_KET_QUA/comparison/` và xuất 5 biểu đồ `.png` trong `06_KET_QUA/figures/`.

## 3. Hướng dẫn Biên dịch và Chạy Tái lập
```bash
# Cách 1: Chạy tự động toàn bộ (biên dịch + mô phỏng S01..S05 + phân tích & vẽ biểu đồ)
bash /home/hhai/BaoCao_NS3/04_SOURCE_CODE/scripts/run_all_experiments.sh

# Cách 2: Biên dịch và chạy từng bước thủ công
cp /home/hhai/BaoCao_NS3/04_SOURCE_CODE/modified/dctcp_experiments.cc /home/hhai/ns-allinone-3.42/ns-3.42/scratch/
cd /home/hhai/ns-allinone-3.42/ns-3.42
./ns3 build dctcp_experiments tcp-validation dctcp-example

# Chạy phân tích và vẽ lại biểu đồ từ dữ liệu CSV đã thu thập
python3 /home/hhai/BaoCao_NS3/04_SOURCE_CODE/scripts/analyze_and_plot.py
```
