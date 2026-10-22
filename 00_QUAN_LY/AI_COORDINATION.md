# AI Coordination Protocol — BaoCao_NS3

## Mục tiêu
Phân tích 2 bài báo, thống nhất mô hình, tái hiện/thực hiện
mô phỏng bằng NS-3.42, kiểm chứng và so sánh kết quả với bài báo.

## Vai trò
- Gemini: điều phối chính, phân tích lý thuyết và bài báo.
- Claude: phản biện độc lập, phân tích kỹ thuật và NS-3.42.
- Người dùng: quyết định cuối cùng, phê duyệt đặc tả và thay đổi lớn.

## Quy tắc phối hợp
1. Đọc tệp này trước khi làm việc.
2. Không tự giả định nội dung bài báo chưa đọc được.
3. Mọi kết luận quan trọng phải có nguồn, số trang, phương trình
   hoặc đường dẫn mã nguồn để kiểm tra.
4. Phân biệt rõ: dữ liệu từ bài báo, suy luận, và kết quả mô phỏng mới.
5. Khi giao việc, ghi nhiệm vụ, đầu vào, đầu ra, tiêu chí nghiệm thu
   và các câu hỏi cần AI kia trả lời.
6. Đọc phản hồi của AI kia, trả lời từng ý và ghi nhận bất đồng.
7. Không tự sửa mã nguồn trong giai đoạn phân tích.
8. Không tự cài phần mềm, xóa dữ liệu, ghi đè tệp gốc hoặc push Git.
9. Chưa thống nhất mô hình, tham số, kịch bản và tiêu chí so sánh
   thì chưa triển khai mô phỏng chính thức.
10. Cập nhật trạng thái công việc và ghi quyết định vào 09_NHAT_KY/.

## Chu trình bắt buộc
ANALYZE -> HANDOFF -> CROSS-REVIEW -> RESOLVE DISAGREEMENTS
-> PROPOSE SPEC -> USER APPROVAL -> IMPLEMENT -> TEST -> REPORT.

## Định dạng mỗi lần bàn giao
- ID nhiệm vụ:
- Người giao / người nhận:
- Mục tiêu:
- Đầu vào:
- Việc cần làm:
- Đầu ra và vị trí tệp:
- Tiêu chí nghiệm thu:
- Câu hỏi cần phản hồi:
- Trạng thái: TODO / IN_PROGRESS / REVIEW / DONE / BLOCKED
