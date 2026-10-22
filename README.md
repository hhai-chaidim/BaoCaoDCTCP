# BaoCao_NS3 — Dự án phân tích hai bài báo và tái hiện trên NS-3

## Mục tiêu
Hoàn thành báo cáo và source code tái hiện mô hình/cải tiến trong hai bài báo, kiểm thử, chạy thí nghiệm và so sánh với kết quả công bố.

## Quy tắc quan trọng
1. `01_TAI_LIEU_GOC/` giữ nguyên bản gốc; không chỉnh sửa PDF gốc.
2. `03_DAC_TA_CHUNG/` là nguồn chuẩn đã thống nhất. Không tự ý thay đổi mô hình/kịch bản nếu chưa ghi quyết định.
3. Không ghi số liệu mô phỏng giả định như kết quả thật. Chỉ đưa kết quả vào `06_KET_QUA/` sau khi có log/dữ liệu thực tế.
4. Mỗi kết quả quan trọng phải truy ngược được tới bài báo, phiên bản code, cấu hình và log.
5. AI không được tuyên bố đã chạy code nếu chưa có bằng chứng chạy thực tế.
6. Tránh để hai AI sửa cùng một tệp cùng lúc. Mỗi tệp có một người/AI phụ trách; thay đổi cần được rà soát.
7. Mọi thay đổi quyết định được ghi vào `09_NHAT_KY/decision_log.md`.

## Quy trình
Đọc tài liệu → phân tích độc lập → đối chiếu → chốt đặc tả → triển khai → kiểm thử → chạy thí nghiệm → so sánh → viết báo cáo.

## Vai trò mặc định
- Gemini: phân tích bài báo, lý thuyết, trích xuất thông số và kết quả công bố.
- Claude: phản biện kỹ thuật, khảo sát/triển khai NS-3, kiểm thử và tái lập.
- Người dùng: phê duyệt đặc tả, chạy xác nhận, quản lý phiên bản và quyết định cuối cùng.

## Trạng thái dự án
Cập nhật trong `00_QUAN_LY/project_status.md`.
