# Source Code và Hướng dẫn Tái lập (NS-3.42)

## 1. Môi trường và Phiên bản
- **Hệ điều hành**: Ubuntu Linux (`x86_64`)
- **Phiên bản NS-3**: `ns-3.42` tại `/home/hhai/ns-allinone-3.42/ns-3.42`
- **Trình biên dịch**: `/usr/bin/c++` (CMake build system qua `./ns3`)
- **Cấu hình IDE (`#include` paths)**:
  - Đã tạo `/home/hhai/BaoCao_NS3/.vscode/c_cpp_properties.json`, `/home/hhai/BaoCao_NS3/.clangd`, `/home/hhai/ns-allinone-3.42/ns-3.42/.vscode/c_cpp_properties.json`, và `/home/hhai/ns-allinone-3.42/ns-3.42/.clangd` trỏ tới `/home/hhai/ns-allinone-3.42/ns-3.42/build/include`.
  - Đã vô hiệu hóa dòng `#include "ns3/visualizer-module.h"` trong `/home/hhai/ns-allinone-3.42/ns-3.42/scratch/myfirst.cc` (do module `visualizer` không bật khi `Python Bindings : OFF`) để lệnh `./ns3 build` biên dịch toàn bộ cây thư mục mà không gặp lỗi.

## 2. Cấu trúc thư mục `04_SOURCE_CODE/`
- `original/`: Bản sao nguyên gốc không chỉnh sửa từ NS-3.42 để đối chiếu:
  - `tcp-dctcp.h`, `tcp-dctcp.cc` (`src/internet/model/`)
  - `tcp-dctcp-test.cc`, `tcp-ecn-test.cc` (`src/internet/test/`)
  - `dctcp-example.cc`, `tcp-validation.cc` (`examples/tcp/`)
- `modified/`:
  - `dctcp_experiments.cc`: Chương trình mô phỏng hợp nhất cho toàn bộ các kịch bản thực nghiệm **S02** (Dumbbell 1 luồng biến thiên RTT & kiểm tra Không-ECN), **S03** (Multi-bottleneck 40 luồng `S1/S2/S3` với tùy chọn `instantK` và `useDctcp`), **S04** (Độ dài hàng đợi răng cưa & CDF tại 1Gbps với 2 và 20 luồng), và **S05** (Tính hội tụ 5 luồng khởi động/kết thúc lệch pha). Bản sao thực thi được đặt tại `/home/hhai/ns-allinone-3.42/ns-3.42/scratch/dctcp_experiments.cc`.
- `scripts/`:
  - `run_all_experiments.sh`: Script tổng hợp chạy tự kiểm chứng `S01` và các kịch bản `S02`–`S05`.
  - `run_remaining_s03_s04_s05.sh`, `rerun_s02.sh`: Các script chạy từng phần phục vụ thu thập dữ liệu.
  - `analyze_and_plot.py`: Script Python (`numpy`, `matplotlib`) xử lý dữ liệu thô trong `06_KET_QUA/raw/`, xuất bảng tổng hợp CSV trong `06_KET_QUA/comparison/` và vẽ 5 biểu đồ chuẩn xuất bản trong `06_KET_QUA/figures/`.

## 3. Lệnh Biên dịch và Thực thi Tái lập
```bash
# 1. Sao chép chương trình thực nghiệm vào scratch của NS-3.42 và biên dịch
cp /home/hhai/BaoCao_NS3/04_SOURCE_CODE/modified/dctcp_experiments.cc /home/hhai/ns-allinone-3.42/ns-3.42/scratch/
cd /home/hhai/ns-allinone-3.42/ns-3.42
./ns3 build dctcp_experiments tcp-validation

# 2. Chạy toàn bộ kịch bản mô phỏng (S01 -> S05)
bash /home/hhai/BaoCao_NS3/04_SOURCE_CODE/scripts/run_all_experiments.sh

# 3. Phân tích dữ liệu và sinh biểu đồ + bảng CSV
python3 /home/hhai/BaoCao_NS3/04_SOURCE_CODE/scripts/analyze_and_plot.py
```
