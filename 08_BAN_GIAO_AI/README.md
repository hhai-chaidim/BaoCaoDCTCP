# Quy trình bàn giao giữa Gemini và Claude

## Gemini → Claude
Đặt vào `GEMINI_TO_CLAUDE/`:
- Phân tích từng bài báo
- Bảng trích dẫn và thông số
- Các điểm chưa rõ
- Đề xuất thuật toán và kịch bản

## Claude → Gemini
Đặt vào `CLAUDE_TO_GEMINI/`:
- Phản biện kỹ thuật
- Khảo sát source code
- Thiết kế triển khai và kiểm thử
- Log thật, dữ liệu thô và kết quả

## Mẫu tên tệp
`YYYY-MM-DD_<noi-dung>_v01.md`

Mỗi bản bàn giao cần có:
1. Mục tiêu
2. Tệp đã đọc
3. Kết quả
4. Bằng chứng/nguồn
5. Giả định và hạn chế
6. Câu hỏi cần AI kia phản hồi
7. Trạng thái: DRAFT / REVIEW / APPROVED

Không ghi mật khẩu, token, khóa API hoặc thông tin riêng tư vào thư mục.
