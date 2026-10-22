# Đặc tả kịch bản mô phỏng — TÀI LIỆU CHUẨN

> Trạng thái: APPROVED — Đã được Người dùng phê duyệt ngày 2026-10-22 để chuyển sang T06.

## Môi trường
- **NS-3 version**: `ns-3.42` (tại `/home/hhai/ns-allinone-3.42/ns-3.42/`)
- **Hệ điều hành**: Linux (Ubuntu)
- **Build profile / compiler**: `./ns3` (CMake build system)
- **Source code gốc**:
  - `/home/hhai/ns-allinone-3.42/ns-3.42/src/internet/model/tcp-dctcp.{h,cc}`
  - `/home/hhai/ns-allinone-3.42/ns-3.42/examples/tcp/dctcp-example.cc`
  - `/home/hhai/ns-allinone-3.42/ns-3.42/examples/tcp/tcp-validation.cc`

## Topology và cấu hình

### 1. Topology A — Single-Bottleneck Validation (Bài báo 2, Hình 8, Bảng 4, tr. 69; `tcp-validation.cc`)
- **Sơ đồ topology**: `Server (Sender) --- [1 Gbps, 1 us] --- WAN Router === [50 Mbps, oneWayDelay] === LAN Router --- [1 Gbps, 1 us] --- Client (Receiver)`
- **Số node**: 2 router + các cặp sender/receiver (1 luồng chính + 1 luồng ping).
- **Tốc độ liên kết**: Nhánh truy cập `1000 Mbps`, cổ chai `50 Mbps`.
- **Độ trễ**: Base RTT $\in \{3\mu\text{s}, 10\text{ ms}, 50\text{ ms}, 80\text{ ms}\}$.
- **Queue / queue discipline**: `RedQueueDisc` (`UseEcn=true`, `QW=1`, `MinTh=MaxTh=17` hoặc `17/34`) và `CoDelQueueDisc` (`UseEcn=true`, `CeThreshold=1ms`).
- **Giao thức**: `TcpDctcp` vs `TcpLinuxReno` (`SegmentSize = 1448 B`, `InitialCwnd = 10`).
- **Thời gian mô phỏng**: `15s` – `40s` (luồng bắt đầu tại `t = 5s`).

### 2. Topology B — Multi-Bottleneck 40 Flows (Bài báo 1, Hình 17, tr. 71; Bài báo 2, Hình 17, Bảng 5, tr. 71–72; `dctcp-example.cc`)
- **Sơ đồ topology**:
  - `S1 (10 nodes)` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `Switch T1`
  - `S2 (20 nodes)` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `Switch T1`
  - `Switch T1` $\xrightarrow{10\text{ Gbps}, 10\mu\text{s}}$ `Switch T2` *(Cổ chai 1: 30 luồng gồm 10 S1 + 20 S2)*
  - `S3 (10 nodes)` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `Switch T2`
  - `Switch T2` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `R1 (1 node)` *(Cổ chai 2: 20 luồng gồm 10 S1 + 10 S3)*
  - `Switch T2` $\xrightarrow{1\text{ Gbps}, 10\mu\text{s}}$ `R2 (20 nodes)` *(Nhận 20 luồng từ S2)*
- **Tổng số node**: 63 nodes (40 senders, 21 receivers, 2 switches T1 & T2), 62 liên kết Point-to-Point.
- **Queue / queue discipline**: `RedQueueDisc` (`UseEcn=true`, `UseHardDrop=false`, `MeanPktSize=1500`, `MaxSize=2666p` $\approx 4\text{MB}$, `QW=1.0`).
- **Lưu lượng ứng dụng**: `OnOffApplication` liên tục (`OnTime=1, OffTime=0`, `DataRate=1Gbps`, `PacketSize=1000B`), `SegmentSize=1448B`, `DelAckCount=2`.
- **Thời gian**: Khởi động `flowStartupWindow = 1s`, hội tụ `convergenceTime = 3s`, đo đạc `measurementWindow = 1s` (Tổng `stopTime = 5s`; có thể khảo sát thêm `convergenceTime = 10s`).

### 3. Topology C — Star / Single-Switch Microbenchmarks (Bài báo 1, Hình 1, 13, 16, tr. 64, 70–71)
- **Sơ đồ topology**: $N$ senders nối vào Switch T1 (`1 Gbps`, delay `10 us`–`25 us`), cùng gửi tới 1 receiver R1 qua cổ chai `1 Gbps` (`RedQueueDisc`, `MaxSize=2666p`, `QW=1.0`, `MinTh=MaxTh=20`).
- **Mục đích**:
  1. So sánh trực tiếp độ dài hàng đợi theo thời gian và CDF giữa **DCTCP** ($K=20$) và **TCP NewReno/LinuxReno** (DropTail / ECN tắt) với $N=2$ và $N=20$ luồng (tái hiện Hình 1 và Hình 13 Bài báo 1).
  2. Kiểm chứng tính hội tụ và công bằng (**Convergence test** — Hình 16 Bài báo 1): 5 luồng khởi động tuần tự cách nhau 30s và dừng tuần tự cách nhau 30s.

## Kịch bản
| ID | Mục đích | Biến thay đổi | Giá trị | Giữ cố định | Số lần chạy/seed |
|---|---|---|---|---|---|
| **S01** | Unit Tests & Smoke Test | Test suite / Thời gian chạy ngắn | `tcp-ecn-test`, `tcp-dctcp-test`, `tcp-validation` (`dctcp-10ms`, `dctcp-80ms`) | Cấu hình chuẩn NS-3.42 | 1 lần (deterministic test) |
| **S02** | Kiểm chứng 1 luồng & Fallback (Bài báo 2, Hình 9–16) | Base RTT & trạng thái ECN | $\text{RTT} \in \{3\mu\text{s}, 10\text{ms}, 50\text{ms}\}$; `UseEcn` $\in \{\text{true}, \text{false}\}$ với `TcpDctcp` vs `TcpLinuxReno` | Bottleneck `50 Mbps`, `SegmentSize=1448B`, `InitialCwnd=10` | Seed = 1 |
| **S03** | Tái hiện Multi-bottleneck 40 luồng (Bài báo 1 Hình 17 & Bài báo 2 Bảng 5) | Giao thức & Ngưỡng đánh dấu RED (`MinTh`/`MaxTh`) | - **S03a**: Mặc định NS-3.42 (`MinTh=20, MaxTh=60` ở 1G; `50/150` ở 10G)<br>- **S03b**: Chuẩn Bài báo 1 (`MinTh=MaxTh=20` ở 1G; `MinTh=MaxTh=65` ở 10G)<br>- **S03c**: Baseline TCP (`enableSwitchEcn=false` / `TcpLinuxReno`) | Topology 63 nodes, `flowStartup=1s`, `convergence=3s`, `measure=1s` | 3 seeds (`RngRun = 1, 2, 3`) |
| **S04** | Tái hiện Queue Length Sawtooth & CDF (Bài báo 1 Hình 1 & Hình 13) | Giao thức (`TcpDctcp` vs `TcpLinuxReno`) & Số luồng $N$ | $N \in \{2, 20\}$ luồng dài hạn | Bottleneck `1 Gbps`, $K = 20$ gói (`MinTh=MaxTh=20, QW=1`) | Seed = 1 |
| **S05** | Tái hiện Hội tụ 5 luồng (Bài báo 1 Hình 16) | Giao thức (`TcpDctcp` vs `TcpLinuxReno`) | 5 luồng bắt đầu lúc $t = 0, 30, 60, 90, 120\text{s}$ và kết thúc lúc $t = 270, 240, 210, 180, 300\text{s}$ | Bottleneck `1 Gbps`, $K = 20$ gói, tổng thời gian `300s` | Seed = 1 |

## Chỉ số cần thu
| Chỉ số | Định nghĩa/công thức | Đơn vị | Nguồn bài báo | Phương pháp đo trong NS-3 |
|---|---|---|---|---|
| **Throughput** | $\frac{\text{Tổng byte nhận tại Sink} \times 8}{\Delta t \times 10^6}$ | Mbps (hoặc Gbps) | Bài báo 1 (Hình 14, 16, 17); Bài báo 2 (Hình 9–11, Bảng 5) | Trace `Rx` trên `PacketSink` trong cửa sổ đo $\Delta t$ |
| **Queue Length / Delay** | Số gói tức thời `qSize = queue->GetNPackets()` và độ trễ $\frac{\text{qSize} \times 1500 \times 8}{C}$ | packets / KB / $\mu\text{s}$ / ms | Bài báo 1 (Hình 1, 12, 13, 15); Bài báo 2 (Hình 9–11, 15–16) | `QueueDisc::GetNPackets()` / trace `PacketsInQueue`, `BytesInQueue` |
| **Congestion Window (`cwnd`)** | Kích thước cửa sổ tắc nghẽn $\frac{\text{m\_cWnd}}{\text{SegmentSize}}$ | segments | Bài báo 1 (Eq. 2, Hình 11); Bài báo 2 (Hình 9–16) | Trace `CongestionWindow` trên `TcpSocketBase` |
| **DCTCP $\alpha$** | Ước lượng tỷ lệ byte bị đánh dấu CE (Eq. 1) | $[0, 1]$ | Bài báo 1 (Eq. 1, tr. 67); Bài báo 2 (Eq. 4, tr. 67) | Trace `CongestionEstimate` trên `TcpDctcp` |
| **Jain's Fairness Index** | $J = \frac{(\sum_{i=1}^n x_i)^2}{n \sum_{i=1}^n x_i^2}$ | $[0, 1]$ | Bài báo 1 (Mục 4.1, tr. 71); Bài báo 2 (Bảng 5, tr. 71) | Tính trực tiếp từ vector bytes nhận của từng nhóm luồng (`PrintFairness`) |

## Quy tắc so sánh
- Chỉ so sánh các chỉ số cùng định nghĩa, đơn vị và điều kiện.
- Đối với Kịch bản **S03 (Multi-bottleneck 40 luồng)**: đối chiếu song song 3 cột số liệu: (1) Kết quả phần cứng Bài báo 1 (46 / 475 / 54 Mbps, $J=0.99$), (2) Kết quả công bố Bài báo 2 (23 / 471 / 74 Mbps, $J=0.99$), và (3) Kết quả thực chạy trên NS-3.42 (cả cấu hình mặc định S03a và cấu hình ngưỡng tức thời chuẩn S03b).
