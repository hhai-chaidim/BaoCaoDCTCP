# Đặc tả mô hình — TÀI LIỆU CHUẨN

> Trạng thái: APPROVED — Đã được Người dùng phê duyệt ngày 2026-10-22 để chuyển sang T06.
> Không điền bằng suy đoán. Ghi nguồn bài báo (trang/mục/hình/bảng/công thức) và mã nguồn NS-3.42 cho từng chi tiết.

## 1. Bài báo và mục tiêu
- **Bài báo 1 (Lý thuyết gốc)**: Mohammad Alizadeh et al., *"Data Center TCP (DCTCP)"*, ACM SIGCOMM 2010, trang 63–74 (`01_TAI_LIEU_GOC/BAI_BAO_1_LY_THUYET/1851182.1851192.pdf`).
- **Bài báo 2 (Thực nghiệm NS-3)**: Vivek Jain, Thomas R. Henderson, Shravya K. S., Mohit P. Tahiliani, *"Data Center TCP in ns-3: Implementation, Validation and Evaluation"*, WNS3 2020, trang 65–72 (`01_TAI_LIEU_GOC/BAI_BAO_2_THUC_NGHIEM/3389400.3389405.pdf`).
- **Ý tưởng chính**: Trích xuất phản hồi tắc nghẽn đa-bit ($\alpha \in [0, 1]$) từ chuỗi đánh dấu ECN 1-bit tại switch với ngưỡng hàng đợi tức thời $K$ rất nhỏ, giúp bên gửi giảm cửa sổ tắc nghẽn (`cwnd`) tỷ lệ thuận với mức độ tắc nghẽn thay vì luôn cắt giảm một nửa như TCP truyền thống (Bài báo 1, Mục 3, tr. 67; Bài báo 2, Mục 2.2, tr. 66–67).
- **Vấn đề cần giải quyết**: Giải quyết 3 suy giảm hiệu năng trong mạng trung tâm dữ liệu dùng switch bộ đệm nông (shallow-buffered switches): (1) *Incast*, (2) *Queue buildup*, (3) *Buffer pressure* — đảm bảo đồng thời độ trễ thấp cho luồng ngắn, khả năng chịu burst cao và thông lượng cao cho luồng dài (Bài báo 1, Mục 1 tr. 63 & Mục 2.3 tr. 65–67).
- **Cải tiến/mô hình đề xuất**: Giao thức DCTCP gồm 3 thành phần phối hợp tại Switch, Receiver và Sender (Bài báo 1, Mục 3.1, tr. 67–68) và mô hình hóa trong NS-3 theo chuẩn RFC 8257 (Bài báo 2, Mục 3, tr. 67–69).

## 2. Thuật toán
- **Đầu vào**:
  - Tại Switch: Gói tin IP đến hàng đợi có độ dài tức thời $Q$ (hoặc `nQueued` gói) và ngưỡng đánh dấu $K$.
  - Tại Receiver: Gói tin dữ liệu nhận được có mã ECN trong IP header (`ECT(0)`, `ECT(1)`, hoặc `CE`).
  - Tại Sender: Các gói ACK nhận được có/không có cờ `ECE`, số byte được xác nhận (`segmentsAcked * segmentSize`), trọng số $g$, ước lượng $\alpha$ hiện tại.
- **Đầu ra**:
  - Tại Switch: Gói tin được đánh dấu mã `CE` (`0b11`) nếu vượt ngưỡng $K$.
  - Tại Receiver: Gói ACK phản hồi chính xác trạng thái `CE` qua cờ `ECE` (gửi ACK tức thời khi trạng thái `CE` đổi từ $0 \leftrightarrow 1$).
  - Tại Sender: Giá trị $\alpha$ cập nhật mỗi cửa sổ quan sát và kích thước `cwnd` / `ssThresh` điều chỉnh tỷ lệ với $\alpha$.
- **Các bước thuật toán & Công thức**:
  1. **Tại Switch (Bài báo 1, Mục 3.1, tr. 67; Bài báo 2, Mục 2.2.1, Eq. 1, tr. 67)**:
     - Đánh dấu `CE` nếu độ dài hàng đợi tức thời vượt ngưỡng $K$, với điều kiện chọn $K$ không gây cạn hàng đợi (Bài báo 1, Eq. 13, tr. 69):
       $$K > \frac{C \times RTT}{7}$$
  2. **Tại Receiver (Bài báo 1, Mục 3.1, Hình 10, tr. 67; Bài báo 2, Mục 3.2.3, Hình 7, tr. 68–69)**:
     - Duy trì biến trạng thái `m_ceState` (`CE=0` hoặc `CE=1`). Khi nhận gói có trạng thái khác với `m_ceState` và đang có Delayed ACK chờ gửi (`m_delayedAckReserved`), lập tức gửi ACK cho số thứ tự trước đó (`m_priorRcvNxt`) với cờ `ECE` tương ứng trước khi cập nhật `m_ceState` (`tcp-dctcp.cc`, dòng 184–240).
  3. **Tại Sender (Bài báo 1, Mục 3.1, Eq. 1–2, tr. 67–68; Bài báo 2, Mục 2.2.3, Eq. 2–4, tr. 67)**:
     - Trong mỗi cửa sổ quan sát (khi `m_lastAckedSeq >= m_nextSeq`), tính tỷ lệ byte bị đánh dấu $F = \frac{\text{m\_ackedBytesEcn}}{\text{m\_ackedBytesTotal}}$ và cập nhật $\alpha$ (`tcp-dctcp.cc`, dòng 152–163):
       $$\alpha \leftarrow (1 - g) \times \alpha + g \times F$$
     - Khi nhận ACK có cờ `ECE` (tối đa 1 lần trong mỗi cửa sổ), giảm cửa sổ tắc nghẽn (`tcp-dctcp.cc`, dòng 132–136):
       $$cwnd \leftarrow cwnd \times \left(1 - \frac{\alpha}{2}\right)$$
     - Khi không có tắc nghẽn (Congestion Avoidance), tăng cộng như TCP chuẩn (Bài báo 2, Eq. 2, tr. 67):
       $$cwnd \leftarrow cwnd + \frac{1}{cwnd}$$
- **Điều kiện biên & Giả định**:
  - Giá trị khởi tạo $\alpha_0 = 1.0$, trọng số $g = 0.0625$ ($1/16$) theo RFC 8257 (Bài báo 2, Mục 3.2.2, tr. 68; `tcp-dctcp.cc` dòng 43–52).
  - Đặt mã `ECT` trên cả gói điều khiển `SYN`, `SYN+ACK`, `RST` và `ACK` đối với lưu lượng DCTCP (Bài báo 2, Mục 3.2.2, tr. 68; `tcp-dctcp-test.cc` dòng 91–147).
  - Giới hạn của mô hình NS-3.42 hiện tại: `TcpDctcp::GetSsThresh()` luôn dùng $(1 - \alpha/2)$ ngay cả khi mất gói (`tcp-dctcp.h` dòng 36–40).

## 3. Tham số
| Tham số | Giá trị | Đơn vị | Nguồn chính xác | Độ chắc chắn |
|---|---|---|---|---|
| `DctcpShiftG` ($g$) | `0.0625` ($1/16$) | không thứ nguyên | Bài báo 1 (tr. 69–70); Bài báo 2 (Mục 3.2.2, tr. 68); `tcp-dctcp.cc:45` | 100% (Xác nhận từ bài báo & code) |
| `DctcpAlphaOnInit` ($\alpha_0$) | `1.0` | không thứ nguyên | Bài báo 2 (Mục 3.2.2, tr. 68); `tcp-dctcp.cc:50` | 100% (Xác nhận từ bài báo & code) |
| `UseEct0` | `true` (`ECT(0)`) | boolean | Bài báo 2 (Mục 3.2.2, tr. 68); `tcp-dctcp.cc:55` | 100% (Xác nhận từ bài báo & code) |
| Ngưỡng $K$ ở 1 Gbps | `20` (Bài báo 1) / `MinTh=20, MaxTh=60` (`dctcp-example.cc`) | packets | Bài báo 1 (Mục 3.5, tr. 69–70); `dctcp-example.cc:337-338, 407-409` | 100% (Cần khảo sát cả 2 cấu hình) |
| Ngưỡng $K$ ở 10 Gbps | `65` (Bài báo 1) / `MinTh=50, MaxTh=150` (`dctcp-example.cc`) | packets | Bài báo 1 (Mục 3.5, tr. 70); `dctcp-example.cc:393-395` | 100% (Cần khảo sát cả 2 cấu hình) |
| Trọng số hàng đợi RED `QW` | `1.0` | không thứ nguyên | Bài báo 2 (Mục 3.2.4, tr. 69); `dctcp-example.cc:336` | 100% (Xác nhận từ bài báo & code) |
| `SegmentSize` | `1448` | bytes | Bài báo 2 (Bảng 4, tr. 69); `dctcp-example.cc:320` | 100% (Xác nhận từ bài báo & code) |
| `DelAckCount` ($m$) | `2` | packets | Bài báo 1 (Chú thích 3, tr. 67); Bài báo 2 (Mục 2.2.2, tr. 67); `dctcp-example.cc:321` | 100% (Xác nhận từ bài báo & code) |

## 4. Ánh xạ sang NS-3.42
| Thành phần bài báo | Thành phần NS-3.42 / source file | Cách hiện thực | Cách kiểm chứng |
|---|---|---|---|
| Sender & Receiver DCTCP | `ns3::TcpDctcp` (`/home/hhai/ns-allinone-3.42/ns-3.42/src/internet/model/tcp-dctcp.{h,cc}`) | Kế thừa `TcpLinuxReno`, cài đặt `Init()`, `GetSsThresh()`, `PktsAcked()`, `CwndEvent()`, `CeState0to1()`, `CeState1to0()` | Unit test `tcp-dctcp-test` & trace `CongestionEstimate`, `CongestionWindow` |
| Đàm phán & Trạng thái ECN | `ns3::TcpSocketBase`, `ns3::TcpSocketState` (`src/internet/model/tcp-socket-base.{h,cc}`, `tcp-socket-state.{h,cc}`) | Quản lý `EcnMode_t::DctcpEcn`, các trạng thái `ECN_IDLE`, `ECN_CE_RCVD`, `ECN_SENDING_ECE`, `ECN_ECE_RCVD`, `ECN_CWR_SENT` | Unit test `tcp-ecn-test` & `tcp-dctcp-test` |
| Đánh dấu ECN tại Switch | `ns3::RedQueueDisc` (`src/traffic-control/model/red-queue-disc.{h,cc}`) hoặc `ns3::CoDelQueueDisc` | Đặt `UseEcn=true`, `UseHardDrop=false`, `QW=1.0`, `MinTh`/`MaxTh` | Trace `PacketsInQueue`, `Mark`, `Drop` trên `QueueDisc` |
| Kịch bản Multi-bottleneck Hình 17 | `examples/tcp/dctcp-example.cc` | 40 luồng (S1: 10, S2: 20, S3: 10) qua T1–T2 (10Gbps) và T2–R1 (1Gbps) | Xuất 6 tệp `.dat` thông lượng, fairness và độ dài hàng đợi |
| Kịch bản Single-flow Validation | `examples/tcp/tcp-validation.cc` | 1 hoặc 2 luồng qua cổ chai 50 Mbps với RTT biến thiên | Chạy cờ `--validate=dctcp-10ms`, `--validate=dctcp-80ms` và xuất trace |

## 5. Điểm chưa rõ / bất đồng
| ID | Vấn đề | Bằng chứng | Phương án | Trạng thái |
|---|---|---|---|---|
| U1 | Lựa chọn bộ kịch bản mô phỏng chính thức | Xem `02_PHAN_TICH/DOI_CHIEU/T03_T04_doi_chieu_NS3_42_va_diem_chua_chac_chan.md` | Chạy cả kịch bản chuẩn NS-3.42 (Bài báo 2) và kịch bản đối chứng ngưỡng $K$ / Queue Length / Convergence (Bài báo 1) | Chờ Người dùng duyệt |
| U2 | Khác biệt `MinTh != MaxTh` trong `dctcp-example.cc` so với `MinTh = MaxTh = K` trong Bài báo 1 | `dctcp-example.cc` dòng 393–409 đặt `50/150` và `20/60` | Khảo sát cả 2 cấu hình để giải thích sự dịch chuyển thông lượng S1 (23 Mbps vs 46 Mbps) và S3 (74 Mbps vs 54 Mbps) | Chờ Người dùng duyệt |
| U3 | `TcpDctcp::GetSsThresh` chưa xử lý riêng sự kiện mất gói theo RFC 8257 Mục 3.5 | `tcp-dctcp.h` dòng 36–40; `tcp.rst` dòng 1036–1041 | Phân tích trong báo cáo; giữ nguyên khi chạy tái hiện chuẩn và đề xuất bản vá kiểm chứng nếu người dùng đồng ý | Chờ Người dùng duyệt |

## 6. Phê duyệt
- Người duyệt: Người dùng
- Ngày: 2026-10-22
- Phiên bản đặc tả: v1.0-APPROVED
