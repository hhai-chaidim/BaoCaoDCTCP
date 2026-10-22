# Đặc tả Kịch bản Mô phỏng (S01 – S05)

## 1. Môi trường Thực nghiệm
- **Phiên bản NS-3**: `ns-3.42` (`/home/hhai/ns-allinone-3.42/ns-3.42/`)
- **Hệ điều hành**: Ubuntu Linux (`x86_64`)
- **Trình biên dịch**: GCC `g++` (CMake/Ninja qua công cụ `./ns3`)
- **Mã nguồn đối chiếu & thực thi**:
  - Mã nguồn gốc NS-3.42: `04_SOURCE_CODE/original/` (`tcp-dctcp.{h,cc}`, `dctcp-example.cc`, `tcp-validation.cc`)
  - Chương trình thực nghiệm hợp nhất: `04_SOURCE_CODE/modified/dctcp_experiments.cc`

---

## 2. Cấu hình Topology

### 2.1. Topology A — Single-Bottleneck Dumbbell (Bài báo 2, Hình 8, Bảng 4, tr. 69)
- **Sơ đồ**: `Sender --- [1 Gbps] --- Router 1 === [50 Mbps, baseRtt] === Router 2 --- [1 Gbps] --- Receiver`
- **Tốc độ liên kết**: Nhánh truy cập `1 Gbps`, liên kết cổ chai `50 Mbps`.
- **Độ trễ khứ hồi (`baseRtt`)**: $\{3\mu\text{s}, 10\text{ ms}, 50\text{ ms}, 80\text{ ms}\}$.
- **Hàng đợi tại cổ chai**: `RedQueueDisc` (`QW=1.0`, `MinTh=17, MaxTh=34` gói, `MaxSize=500p`) hoặc `CoDelQueueDisc` (`CeThreshold=1ms` trong `tcp-validation.cc`).
- **Giao thức**: `TcpDctcp` và `TcpLinuxReno` (`SegmentSize = 1448 B`, `InitialCwnd = 10`, `EnablePacing = true`).
- **Thời gian mô phỏng**: `15s` (và `40s` cho trường hợp validation `80ms`).

### 2.2. Topology B — Multi-Bottleneck 40 Flows (Bài báo 1, Hình 17, tr. 73; Bài báo 2, Hình 17, Bảng 5, tr. 71–72)
- **Sơ đồ kết nối (63 nút, 62 liên kết Point-to-Point, độ trễ mỗi chặng `10 us`)**:
  - `S1 (10 senders)` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `Switch T1`
  - `S2 (20 senders)` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `Switch T1`
  - `Switch T1` $\xrightarrow{10\text{ Gbps}, 10\mu\text{s}}$ `Switch T2` *(Cổ chai 1: 30 luồng gồm 10 luồng S1 + 20 luồng S2)*
  - `S3 (10 senders)` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `Switch T2`
  - `Switch T2` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `R1 (1 receiver)` *(Cổ chai 2: 20 luồng gồm 10 luồng S1 + 10 luồng S3)*
  - `Switch T2` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `R2 (20 receivers)` *(Nhận 20 luồng từ S2)*
- **Hàng đợi tại các cổng**: `RedQueueDisc` (`UseEcn=true`, `UseHardDrop=false`, `MeanPktSize=1500`, `MaxSize=2666p` $\approx 4\text{MB}$, `QW=1.0`).
- **Lưu lượng ứng dụng**: `OnOffApplication` phát liên tục (`OnTime=1, OffTime=0`, `DataRate=1Gbps`, `PacketSize=1000B`), `SegmentSize=1448B`, `DelAckCount=2`.
- **Thời gian mô phỏng**: `flowStartupWindow = 1s`, `convergenceTime = 3s`, `measurementWindow = 1s` (tổng `stopTime = 5s`).

### 2.3. Topology C — Star / Single-Switch 1Gbps Microbenchmarks (Bài báo 1, Hình 1, 13, 16, tr. 64, 71–72)
- **Sơ đồ kết nối**: $N$ máy gửi nối vào `Switch` (`1 Gbps`, delay `25 us`), cùng gửi tới `Receiver` qua liên kết cổ chai `1 Gbps`, delay `25 us` ($\text{RTT} = 100\mu\text{s}$).
- **Hàng đợi tại cổ chai**: `RedQueueDisc` (`MaxSize=466p` $\approx 699\text{ KB}$, `QW=1.0`, `MinTh=MaxTh=K=20` gói khi bật ECN; DropTail ở `466p` khi tắt ECN).

---

## 3. Danh mục Kịch bản Thực nghiệm

| ID | Mục đích | Biến khảo sát | Giá trị tham số | Cấu hình cố định | Số lần chạy / Seed |
|---|---|---|---|---|---|
| **S01** | Tự kiểm chứng chuẩn NS-3 (`tcp-validation`) | Thời gian khứ hồi (`baseRtt`) | `dctcp-10ms` (`15s`), `dctcp-80ms` (`40s`) | Cổ chai `50 Mbps`, `CoDelQueueDisc` (`CeThreshold=1ms`) | 1 lần (tất định) |
| **S02** | Đơn luồng biến thiên RTT & Kiểm tra Không-ECN (Bài báo 2, Hình 9–16) | `baseRtt` & trạng thái `enableSwitchEcn` | $\text{RTT} \in \{3\mu\text{s}, 10\text{ms}, 50\text{ms}\}$ (`ECN=true`); $\text{RTT}=10\text{ms}$ (`ECN=false`) với `TcpDctcp` và `TcpLinuxReno` | Cổ chai `50 Mbps`, `MinTh=17, MaxTh=34`, `stopTime=15s` | `RngRun = 1` |
| **S03** | Đa cổ chai 40 luồng (Bài báo 1 Hình 17 & Bài báo 2 Bảng 5) | Giao thức & Ngưỡng đánh dấu RED (`MinTh`/`MaxTh`) | - **S03a**: Mặc định NS-3.42 (`20/60` ở 1G; `50/150` ở 10G)<br>- **S03b**: Tức thời Bài báo 1 (`MinTh=MaxTh=20` ở 1G; `65` ở 10G)<br>- **S03c**: Đối chứng `TcpLinuxReno` (`ECN=false`) | Topology 63 nút, `startup=1s`, `convergence=3s`, `measure=1s` (`stopTime=5s`) | 3 seeds (`RngRun = 1, 2, 3`) + 1 lượt lặp lại `seed1` |
| **S04** | Dao động hàng đợi răng cưa & CDF (Bài báo 1 Hình 1 & Hình 13) | Giao thức (`TcpDctcp` vs `TcpLinuxReno`) & Số luồng $N$ | $N \in \{2, 20\}$ luồng dài hạn | Cổ chai `1 Gbps`, $K = 20$ gói (`MinTh=MaxTh=20`), `MaxSize=466p`, `stopTime=5s` | `RngRun = 1` |
| **S05** | Hội tụ động 5 luồng (Bài báo 1 Hình 16) | Giao thức (`TcpDctcp` vs `TcpLinuxReno`) | 5 luồng gia nhập lúc $t = 0, 3, 6, 9, 12\text{s}$ và rời mạng lúc $t = 30, 27, 24, 21, 18\text{s}$ | Cổ chai `1 Gbps`, $K = 20$ gói, `stepTime=3s`, tổng thời gian `30s` | `RngRun = 1` |

---

## 4. Các Chỉ số Đo đạc và Đánh giá
| Chỉ số | Định nghĩa / Công thức | Đơn vị | Nguồn tham chiếu | Phương pháp thu thập trong NS-3 |
|---|---|---|---|---|
| **Throughput** | $\frac{\text{Tổng byte nhận tại Sink} \times 8}{\Delta t \times 10^6}$ | Mbps / Gbps | Bài báo 1 (Hình 16, 17); Bài báo 2 (Hình 9–11, Bảng 5) | Trace `Rx` trên `PacketSink` trong cửa sổ đo $\Delta t$ |
| **Queue Length / Delay** | Số gói tức thời `qDisc->GetNPackets()` và độ trễ $\frac{\text{bytes} \times 8}{C}$ | packets / KB / $\mu\text{s}$ / ms | Bài báo 1 (Hình 1, 13); Bài báo 2 (Hình 9–11) | Lấy mẫu định kỳ `QueueDisc::GetNPackets()` và `GetNBytes()` |
| **Congestion Window (`cwnd`)** | Kích thước cửa sổ tắc nghẽn $\frac{\text{m\_cWnd}}{\text{SegmentSize}}$ | segments | Bài báo 1 (Eq. 2); Bài báo 2 (Hình 9–16) | Trace `CongestionWindow` trên `TcpSocketBase` |
| **DCTCP $\alpha$** | Ước lượng tỷ lệ byte bị đánh dấu CE (Eq. 1) | $[0, 1]$ | Bài báo 1 (Eq. 1, tr. 66); Bài báo 2 (Eq. 4, tr. 67) | Trace `CongestionEstimate` trên `TcpDctcp` |
| **Jain's Fairness Index** | $J = \frac{(\sum_{i=1}^n x_i)^2}{n \sum_{i=1}^n x_i^2}$ | $[0, 1]$ | Bài báo 1 (Mục 4.1, tr. 71); Bài báo 2 (Bảng 5, tr. 71) | Tính trực tiếp từ thông lượng của các luồng trong cùng nhóm |
