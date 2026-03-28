# Quy tắc phát triển dự án

## 1. Mô hình lập trình 

- **Cấm sử dụng Lập trình Hướng đối tượng (No OOP):** 
  - Dự án này **không** sử dụng các đặc tính của OOP như OOP classes, kế thừa (inheritance), đa hình (polymorphism)...
  - Áp dụng phương pháp **Lập trình Cấu trúc/Thủ tục (Procedural Programming)**.
  - Sử dụng `struct` thuần túy để nhóm dữ liệu. Các hàm xử lý (functions) phải được viết tách biệt khỏi cấu trúc dữ liệu, nhận tham số là `struct` thay vì đóng gói thành phương thức (methods) bên trong class.

## 2. Quy tắc đặt tên

- **Biến số và Hàm:** 
  - Bắt buộc phải sử dụng định dạng `snake_case`.
  - ✅ Hợp lệ: `student_name`, `calculate_average_score()`, `print_main_menu()`.
  - ❌ Không hợp lệ: `studentName` (camelCase), `CalculateAverageScore` (PascalCase).
- **Hằng số và Macros:** 
  - Ghi hoa toàn bộ và phân cách bằng gạch dưới (`UPPER_SNAKE_CASE`).
  - ✅ Hợp lệ: `MAX_STUDENTS`, `PI_VALUE`.
- **Cấu trúc (Structs):**
  - Có thể sử dụng `PascalCase` hoặc `snake_case` (Nên thống nhất một cách viết, ví dụ: `student_info`).

## 3. Giao diện người dùng

- **Đồng nhất thư viện UI:** 
  - Toàn bộ các module trong dự án phải dùng **chung một thư viện/framework UI** duy nhất là FTXUI.
- **Quy tắc Thao tác không dùng ID (No Manual ID Input):**
  - **Tuyệt đối cấm** việc lập trình giao diện bắt người dùng/nhân viên phải tự gõ các mã số ID (như ID Truyện, ID Khách hàng, ID Phiếu thuê...) bằng tay để thực hiện các chức năng như Sửa, Xóa, Cho thuê hoặc Trả sách.
  - Phải ứng dụng cơ chế **Tìm kiếm** (ví dụ: tìm qua Tên truyện hoặc Số điện thoại) kết hợp với **Danh sách trực diện** (VD: Component Menu của FTXUI) để người dùng chọn. Trách nhiệm xử lý ID phải thuộc về hệ thống ngầm bên dưới.

## 4. Thao tác Cơ sở Dữ liệu (File Nhị phân)

- **Bảo toàn dữ liệu bằng Xóa mềm (Soft Delete):**
  - Trong mọi quá trình làm nhiệm vụ "Xóa" một thư mục hay thực thể (v.d. Truyện, Phiếu Thuê), các thành viên **thuần túy dùng Soft Delete** (thay đổi trạng thái bộ cờ như `is_deleted = 1` bên trong `struct`).
  - **Tuyệt đối cấm can thiệp thay đổi cấu trúc/kích thước File (Hard Delete)** - tránh mọi nguy cơ gây ra hiện tượng Lỗi khóa ngoại ngầm (Orphaned records) giữa thư viện `comics`.dat` và `rentals`.dat`.

## 5. Quy trình phát triển dự án (Đặc biệt lưu ý)

**Đặc biệt lưu ý: Mỗi lần mở máy tính lên để bắt đầu làm bất cứ cái gì thì phải fetch và pull code về nhánh `main`**

- **Bước 1:** Clone repo, vào nhánh `main` sau đó fetch hoặc pull code về. Nhớ kiểm tra xem ở IDE (VS Code, Antigravity) có đang hiển thị đúng nhánh chưa. Nếu đúng nhánh rồi thì xem có phải code mới nhất không, nếu chưa phải code mới thì phải pull về tới khi nó hiển thị đúng code mới trên nhánh `main`.
- **Bước 2:** Tạo nhánh mới từ nhánh `main`, tiến hành code, commit, và push nhánh mới lên GitHub.
- **Bước 3:** Sau 1 hay nhiều lần push, khi đã làm xong 1 chức năng hay 1 task (không quá nhỏ) thì tạo Pull Request từ nhánh cũ (nhánh vừa tạo) sang nhánh `main`. Nhắn tin thông báo cho Khiêm và đợi phản hồi. *(Lưu ý: Trong lúc đợi phản hồi thì ĐỪNG code thêm bất kỳ cái gì)*.
- **Bước 4:** Sau khi Khiêm phản hồi, tiến hành xóa nhánh cũ ở GitHub Desktop đi, rồi quay lại Bước 1 để bắt đầu luồng làm việc cho tính năng mới.
