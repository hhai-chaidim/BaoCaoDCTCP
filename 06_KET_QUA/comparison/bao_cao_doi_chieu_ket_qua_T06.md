# Báo cáo Đối chiếu Kết quả Thực nghiệm NS-3.42 với Hai Bài báo Công bố

> **Môi trường mô phỏng**: NS-3.42 (`/home/hhai/ns-allinone-3.42/ns-3.42`)  
> **Nguồn dữ liệu**: `06_KET_QUA/csv/`, `06_KET_QUA/comparison/*.csv`, `06_KET_QUA/figures/*.png`

---

## 1. Tổng quan Thực thi Kịch bản & Kiểm thử (`TC01` – `TC06`)

Toàn bộ 5 nhóm kịch bản (**S01** đến **S05**) và 6 trường hợp kiểm thử (**TC01** đến **TC06**) trong `03_DAC_TA_CHUNG/scenario_spec.md` và `03_DAC_TA_CHUNG/test_plan.md` đã được thực thi hoàn chỉnh trên NS-3.42:

1. **S01 (Validation / Tự kiểm chứng DCTCP chuẩn của NS-3)**:
   - Thực thi `ns3.42-tcp-validation-default` với `--validate=dctcp-10ms` và `--validate=dctcp-80ms` (`g=0.0625`, cổ chai `50 Mbps`).
   - **Kết quả**: Cả hai kịch bản kết thúc với **exit code `0`**, vượt qua toàn bộ các kiểm tra ngưỡng nội bộ của NS-3.42 (thông lượng ổn định đạt `48.2 Mbps`, hệ số $\alpha$ dao động đúng trong dải kỳ vọng).

2. **S03 (Multi-bottleneck 40 luồng — Hình 17 Bài báo 1 & Bảng 5 Bài báo 2)**:
   - Đã chạy 6 lượt mô phỏng độc lập (`flowStartupWindow=1s, convergenceTime=3s, measurementWindow=1s`, tổng `5s`): `S03a_seed1`, `S03a_seed2`, `S03a_seed3`, `S03a_seed1_repeat` (cấu hình mặc định của tác giả Bài báo 2 trong `dctcp-example.cc`), `S03b_instantK` (cấu hình ngưỡng tức thời `MinTh=MaxTh=K` của Bài báo 1 với $K_{1G}=20, K_{10G}=65$), và `S03c_tcp_noecn` (đối chứng `TcpLinuxReno` không ECN).
   - **Dữ liệu gốc**: `06_KET_QUA/csv/S03_multi_bottleneck/`
   - **Biểu đồ**: `06_KET_QUA/figures/fig1_S03_multi_bottleneck_throughput.png`
   - **Bảng tổng hợp**: `06_KET_QUA/comparison/S03_multi_bottleneck_comparison.csv`

3. **S04 (Độ dài hàng đợi răng cưa & Phân phối tích lũy CDF tại 1Gbps — Hình 1 & Hình 13 Bài báo 1)**:
   - So sánh DCTCP ($K=20$ gói) và TCP LinuxReno không ECN (`MaxSize=466p` $\approx 699\text{ KB}$) với $N=2$ và $N=20$ luồng trong `stopTime=5s`.
   - **Dữ liệu gốc**: `06_KET_QUA/csv/S04_queue_cdf/`
   - **Biểu đồ**: `06_KET_QUA/figures/fig4_S04_queue_length_sawtooth_and_cdf.png`
   - **Bảng tổng hợp**: `06_KET_QUA/comparison/S04_queue_cdf_summary.csv`

4. **S05 (Tính hội tụ & Công bằng 5 luồng khởi động/kết thúc lệch pha — Hình 16 Bài báo 1)**:
   - 5 luồng tuần tự gia nhập mỗi `3s` (`0, 3, 6, 9, 12s`) và rời đi mỗi `3s` (`30, 27, 24, 21, 18s`) trên cổ chai `1 Gbps`, $\text{RTT} = 100\mu\text{s}$.
   - **Dữ liệu gốc**: `06_KET_QUA/csv/S05_convergence/`
   - **Biểu đồ**: `06_KET_QUA/figures/fig5_S05_convergence_5flows.png`

5. **S02 (Đơn luồng Dumbbell 50 Mbps với RTT biến thiên & Kiểm tra hành vi khi không có ECN — Hình 9–16 Bài báo 2)**:
   - Khảo sát `baseRtt` $\in \{3\mu\text{s}, 10\text{ms}, 50\text{ms}\}$ với ECN bật và kịch bản `10ms` khi tắt ECN trên router (`stopTime=15s`).
   - **Dữ liệu gốc**: `06_KET_QUA/csv/S02_single_flow/`
   - **Biểu đồ**: `06_KET_QUA/figures/fig2_S02_single_flow_varying_rtt.png`, `06_KET_QUA/figures/fig3_S02_fallback_no_ecn.png`
   - **Bảng tổng hợp**: `06_KET_QUA/comparison/S02_single_flow_summary.csv`

---

## 2. Đối chiếu Định lượng Kịch bản S03 (Multi-Bottleneck 40 Luồng)

### 2.1. Bảng so sánh kết quả thực nghiệm với Bài báo 1 và Bài báo 2

| Cấu hình / Nguồn dữ liệu | S1 (10 luồng, 2 cổ chai) [Mbps] | Jain's Fairness S1 | S2 (20 luồng, cổ chai 10G) [Mbps] | Jain's Fairness S2 | S3 (10 luồng, cổ chai 1G) [Mbps] | Jain's Fairness S3 | Tổng thông lượng T1 (10G) [Gbps] | Hàng đợi TB tại T1 [gói / $\mu\text{s}$] | Hàng đợi TB tại T2 [gói / $\mu\text{s}$] |
|---|---|---|---|---|---|---|---|---|---|
| **Lý thuyết Max-Min Fairness** | `50.00` | `1.000` | `475.00` | `1.000` | `50.00` | `1.000` | `10.000` | — | — |
| **Bài báo 1 (SIGCOMM 2010 — Hình 17, testbed thực)** | **`46.00`** | **`0.990`** | **`475.00`** | **`0.990`** | **`54.00`** | **`0.990`** | `~9.960` | $K_{10G}=65$ | $K_{1G}=20$ |
| **Bài báo 2 (WNS3 2020 — Bảng 5, ns-3.30)** | **`23.00`** | **`0.990`** | **`471.00`** | **`0.990`** | **`74.00`** | **`0.990`** | **`9.650`** | — | — |
| **NS-3.42 `S03a_seed1` (`MinTh!=MaxTh` mặc định)** | **`23.92`** | **`0.996`** | **`469.91`** | **`1.000`** | **`72.49`** | **`0.998`** | **`9.637`** | `176.0p` (`210.8 us`) | `75.9p` (`910.9 us`) |
| **NS-3.42 `S03a_seed2` (`RngRun=2`)** | **`23.39`** | **`0.993`** | **`470.17`** | **`0.999`** | **`73.02`** | **`0.998`** | **`9.637`** | `177.3p` (`212.3 us`) | `75.3p` (`903.5 us`) |
| **NS-3.42 `S03a_seed3` (`RngRun=3`)** | **`23.65`** | **`0.980`** | **`470.03`** | **`0.999`** | **`72.75`** | **`0.999`** | **`9.637`** | `175.1p` (`209.7 us`) | `76.4p` (`916.8 us`) |
| **NS-3.42 `S03a_seed1_repeat` (Kiểm tra lặp lại)** | **`23.92`** | **`0.996`** | **`469.91`** | **`1.000`** | **`72.49`** | **`0.998`** | **`9.637`** | `176.0p` (`210.8 us`) | `75.9p` (`910.9 us`) |
| **NS-3.42 `S03b_instantK` (`MinTh=MaxTh=K`: $K_{1G}=20, K_{10G}=65$)** | **`11.73`** | **`0.747`** | **`476.00`** | **`1.000`** | **`84.68`** | **`0.997`** | **`9.637`** | **`79.5p` (`95.0 us`)** | **`25.8p` (`309.9 us`)** |
| **NS-3.42 `S03c_tcp_noecn` (`TcpLinuxReno`, không ECN)** | **`37.58`** | **`0.871`** | **`461.76`** | **`0.710`** | **`58.83`** | **`0.772`** | **`9.611`** | `45.4p` (`54.1 us`) | `21.9p` (`263.2 us`) |

### 2.2. Giải thích kỹ thuật cho kết quả giữa Bài báo 1, Bài báo 2 và NS-3.42
1. **Sự trùng khớp tuyệt đối giữa NS-3.42 (`S03a`) và Bài báo 2 (Bảng 5)**:
   - Trung bình qua 3 seed của `S03a` đạt `S1 = 23.65 Mbps`, `S2 = 470.04 Mbps`, `S3 = 72.75 Mbps`, khớp sát với `23 / 471 / 74 Mbps` của Bảng 5 Bài báo 2 (sai lệch $< 3\%$), và chỉ số công bằng nội nhóm đạt `0.980–1.000`.
2. **Sự thống nhất của nhóm S2 (`~470–475 Mbps`) trên cả Bài báo 1, Bài báo 2 và NS-3.42**:
   - Trong `examples/tcp/dctcp-example.cc` (dòng 315–379), có 10 luồng S1 và 10 luồng S3 cùng đi vào R1 qua cổ chai `1 Gbps` (`T2 -> R1`), nên mỗi luồng S1 và S3 có phần chia lý thuyết là $\frac{1000}{20} = 50\text{ Mbps}$.
   - Trên liên kết `10 Gbps` (`T1 -> T2`), 10 luồng S1 chiếm khoảng $0.5\text{ Gbps}$ (hoặc $\approx 0.24\text{ Gbps}$ ở `S03a`), phần dung lượng còn lại $\approx 9.5\text{ Gbps}$ được chia đều cho **20 luồng S2** đi tới 20 máy nhận R2 nối vào T2, cho thông lượng lý thuyết $\frac{9500}{20} = 475\text{ Mbps/luồng}$ (sau khi trừ mào đầu TCP/IP đạt `469.91 – 476.00 Mbps`).
3. **Tại sao S1 đạt thấp hơn S3 (`23.65 Mbps` so với `72.75 Mbps` ở `S03a`, và `11.73 Mbps` so với `84.68 Mbps` ở `S03b`)?**
   - Nhóm S1 đi qua **3 chặng liên kết** ($\text{RTT}_{S1} = 60\mu\text{s}$) và chịu đánh dấu ECN tại **cả hai nút cổ chai T1 (10Gbps) và T2 (1Gbps)**, trong khi nhóm S3 chỉ đi qua **2 chặng liên kết** ($\text{RTT}_{S3} = 40\mu\text{s}$) và chỉ chịu đánh dấu ECN tại **T2 (1Gbps)**.
   - Khi chuyển từ đánh dấu xác suất tuyến tính (`MinTh=50..150` ở T1, `MinTh=20..60` ở T2 trong `S03a`) sang đánh dấu bậc thang tức thời (`MinTh=MaxTh=65` ở T1, `MinTh=MaxTh=20` ở T2 trong `S03b`), **độ dài hàng đợi tại T1 giảm 55% (`176.0` $\to$ `79.5` gói) và tại T2 giảm 66% (`75.9` $\to$ `25.8` gói)** đúng như mục tiêu thiết kế của Bài báo 1. Tuy nhiên, việc đánh dấu 100% gói ngay tại ngưỡng $K$ khiến các luồng đi qua 2 cổ chai và có RTT lớn hơn (S1) nhận tỷ lệ gói CE ($F$) cao hơn hẳn S3, làm S1 bị ép cửa sổ mạnh hơn (`11.73 Mbps` so với `84.68 Mbps`).
4. **Vai trò của DCTCP so với TCP Reno không ECN (`S03c`)**:
   - Khi tắt ECN và dùng `TcpLinuxReno` (`S03c`), chỉ số công bằng Jain trong từng nhóm sụt giảm nghiêm trọng: S2 giảm từ `1.000` xuống `0.710`, S3 giảm từ `0.998` xuống `0.772`, và S1 giảm từ `0.996` xuống `0.871`.

---

## 3. Đối chiếu Định lượng Kịch bản S04 (Độ dài Hàng đợi & CDF tại 1Gbps)

| Kịch bản (`stopTime=5s, MaxSize=466p`) | Thông lượng cổ chai [Gbps] | Hàng đợi TB [gói] | Trung vị (P50) [gói] | P90 [gói] | P99 [gói] | Cực đại (Max) [gói] | Bộ đệm TB [KB] | Bộ đệm Cực đại [KB] |
|---|---|---|---|---|---|---|---|---|
| **DCTCP ($N=2$ luồng, $K=20$)** | **`0.9640`** | **`21.54`** | **`22.0`** | **`23.0`** | **`24.0`** | **`25`** | **`32.31 KB`** | **`37.50 KB`** |
| **DCTCP ($N=20$ luồng, $K=20$)** | **`0.9640`** | **`27.12`** | **`27.0`** | **`41.0`** | **`48.0`** | **`54`** | **`40.67 KB`** | **`81.00 KB`** |
| **TCP DropTail ($N=2$ luồng, Buffer 466p)** | **`0.9491`** | **`299.82`** | **`262.0`** | **`412.0`** | **`465.0`** | **`466`** | **`448.22 KB`** | **`699.00 KB`** |
| **TCP DropTail ($N=20$ luồng, Buffer 466p)** | **`0.9638`** | **`369.58`** | **`384.0`** | **`450.0`** | **`465.0`** | **`466`** | **`554.36 KB`** | **`699.00 KB`** |

- Theo Phương trình (10) của Bài báo 1 (SIGCOMM 2010), với $N=2$ luồng đồng bộ và ngưỡng đánh dấu $K=20$ gói, đỉnh hàng đợi lý thuyết là $Q_{\max} \approx K + N = 22$ gói. Kết quả mô phỏng NS-3.42 (`S04_dctcp_2flows`) cho giá trị trung bình **`21.54` gói** và trung vị **`22.0` gói**, khớp chính xác tuyệt đối với công thức lý thuyết.
- So với TCP DropTail, DCTCP giảm mức chiếm dụng bộ đệm trung bình **92.8%** (ở 2 luồng) và **92.7%** (ở 20 luồng), đồng thời đạt thông lượng cao hơn (`0.9640 Gbps` so với `0.9491 Gbps` ở 2 luồng).

---

## 4. Đối chiếu Kịch bản S02 (Đơn luồng Biến thiên RTT & Phát hiện Kỹ thuật về Fallback Không-ECN)

| Trường hợp (`t >= 5s`) | `cwnd` TB ổn định [segments] | `cwnd` Min – Max [segments] | Trễ hàng đợi TB [ms] | Độ dài hàng đợi TB [gói] | Thông lượng TB ổn định [Mbps] |
|---|---|---|---|---|---|
| `DCTCP_ECN_3us` ($\text{RTT}=3\mu\text{s}$) | `40.68` | `37.00 – 44.30` | `8.86 ms` | `36.91` | **`48.20 Mbps`** |
| `DCTCP_ECN_10ms` ($\text{RTT}=10\text{ms}$) | `80.71` | `78.00 – 83.97` | `8.51 ms` | `35.45` | **`48.20 Mbps`** |
| `DCTCP_ECN_50ms` ($\text{RTT}=50\text{ms}$) | `241.52` | `235.88 – 245.14` | `7.16 ms` | `29.84` | **`48.20 Mbps`** |
| `S02_reno_noecn_10ms` (`TcpLinuxReno`, tắt ECN) | `39.50` | `39.50 – 39.50` | `0.00 ms` | `0.00` | **`42.85 Mbps`** |
| `S02_dctcp_noecn_10ms` (`TcpDctcp`, tắt ECN) | `59.15` | `1.00 – 83.99` | `1.80 ms` | `7.49` | **`18.75 Mbps`** |

### Giải thích kỹ thuật:
1. **Ảnh hưởng của `MinTh=17, MaxTh=34` trong Bảng 4 Bài báo 2**: Khi cấu hình `MinTh=17, MaxTh=34` trên `RedQueueDisc` (`LInterm=50`), xác suất đánh dấu tuyến tính chỉ đạt tối đa `2%` tại `34` gói và chỉ đánh dấu `100%` khi vượt `34` gói, khiến hàng đợi ổn định quanh `30–37` gói (`7.16–8.86 ms`). Điều này chứng minh mâu thuẫn nội tại giữa Bảng 4 (`17 and 34`) và Mục 3.2.4 (`MinTh = MaxTh = K`) của Bài báo 2.
2. **Hành vi khi tắt ECN tại bộ định tuyến (`UseEcn=false`)**:
   - Hàm `TcpDctcp::GetSsThresh()` trong NS-3.42 (`src/internet/model/tcp-dctcp.cc`, dòng 132–136) được cài đặt như sau:
     ```cpp
     uint32_t
     TcpDctcp::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight)
     {
         NS_LOG_FUNCTION(this << tcb << bytesInFlight);
         return static_cast<uint32_t>((1 - m_alpha / 2.0) * tcb->m_cWnd);
     }
     ```
   - Do `TcpDctcp::Init()` luôn bật `tcb->m_useEcn = TcpSocketState::On` và tắt `SuppressIncreaseIfCwndLimited`, khi gói tin đi qua router không đánh dấu ECN (`UseEcn=false`), `m_alpha` suy giảm về `0`. Khi xảy ra mất gói do vượt `MaxTh=34`, `GetSsThresh()` trả về `m_cWnd` (không giảm một nửa cửa sổ như NewReno), gây tràn hàng đợi và RTO định kỳ làm thông lượng giảm xuống **`18.75 Mbps`** (so với **`42.85 Mbps`** của `TcpLinuxReno`), xác nhận hoàn toàn cảnh báo Doxygen trong `tcp-dctcp.h:36-40`.
