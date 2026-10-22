# Kế hoạch kiểm thử

| ID | Loại kiểm thử | Thao tác | Kết quả mong đợi | Bằng chứng cần lưu | Trạng thái |
|---|---|---|---|---|---|
| TC01 | Kiểm tra build | Build chương trình | Build hoàn tất không lỗi | Lệnh và log build | Chưa chạy |
| TC02 | Smoke test | Chạy kịch bản nhỏ | Chương trình kết thúc, output hợp lệ | Log, exit status | Chưa chạy |
| TC03 | Kiểm tra cấu hình | Xác minh topology/tham số | Khớp đặc tả | Log cấu hình | Chưa chạy |
| TC04 | Kiểm tra dữ liệu | Kiểm tra CSV/log | Đủ cột, đơn vị, không thiếu bất thường | CSV và script kiểm tra | Chưa chạy |
| TC05 | Kiểm tra lặp lại | Chạy lại cùng cấu hình/seed | Kết quả nhất quán theo kỳ vọng | Hai bộ kết quả | Chưa chạy |
| TC06 | Kiểm tra tính hợp lý | Soát throughput/delay/loss | Không vi phạm giới hạn vật lý/logic | Bảng phân tích | Chưa chạy |

Không đánh dấu PASS nếu chưa có bằng chứng.
