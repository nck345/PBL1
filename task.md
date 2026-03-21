# Danh sách nhiệm vụ dự án PBL1
*(⚠️ Tuân thủ nghiêm ngặt tiêu chuẩn `rule.md`: Lập trình thủ tục - Không dùng Class/OOP, biến & hàm ở dạng `snake_case`, toàn bộ giao diện dùng thư viện **FTXUI**)*

## 👨‍💻 Nhiệm vụ của Khiêm
**Vai trò: Kỹ sư Dữ liệu Truyện & Nền tảng Core**

### 1. Thiết kế Cấu trúc (Models)
- [ ] Thiết lập `struct` thuần túy (Tuyệt đối KHÔNG dùng Class hay khai báo hàm trong struct) trong `models/Comic.h` **(~3% công việc)**
- [ ] Định nghĩa các thuộc tính của truyện với chuẩn `snake_case` (ví dụ: `id`, `comic_name`, `price`, `quantity`, `is_deleted` để đánh dấu xóa mềm) **(~3% công việc)**
- [ ] Thiết lập `struct` ngày tháng trong `models/Date.h` (Không chứa các method `set`/`get`) **(~2% công việc)**

### 2. Tiện ích (Utils)
- [ ] Khởi tạo và viết tiện ích `utils/InputHandler` (Validate chống lỗi nhập chữ vào ô số, chống trôi lệnh...) **(~8% công việc)**
- [ ] Khởi tạo và viết tiện ích `utils/StringUtils.h` (Cắt khoảng trắng dư thừa, Format hiển thị tiền tệ VND...) **(~7% công việc)**

### 3. Xử lý File nhị phân (Repository)
- [ ] Khởi tạo khung file `repository/ComicRepo.cpp` **(~2% công việc)**
- [ ] Viết hàm **Đọc toàn bộ danh sách Truyện** (Load từ `comics.dat` ra mảng dữ liệu để phục vụ UI in bảng) **(~5% công việc)**
- [ ] Viết hàm đọc/ghi `metadata.dat` để lấy ID tự động tăng (Cấp ID độc nhất mỗi khi thêm truyện mới) **(~8% công việc)**
- [ ] Viết hàm **Thêm mới truyện** và cấu hình ghi dữ liệu xuống file `comics.dat` (kết hợp lấy ID tự sinh) **(~10% công việc)**
- [ ] Viết hàm **Sửa thông tin truyện** (Đọc, tìm kiếm, cập nhật và lưu lại vào file) **(~8% công việc)**
- [ ] Viết hàm **Xóa truyện** (Triển khai theo logic Soft Delete - đánh dấu trạng thái xóa chứ không xóa vật lý) **(~2% công việc)**
- [ ] Viết hàm **Tìm kiếm Truyện** (Quét file `comics.dat` để lấy dữ liệu) **(~5% công việc)**

### 4. Giao diện (UI)
- [ ] Khởi tạo tính năng UI bằng thư viện **FTXUI** vào dự án **(~5% công việc)**
- [ ] Dùng Component `Table` của tài liệu FTXUI để in ấn màn hình theo chuẩn **(~12% công việc)**
- [ ] Dùng Component của FTXUI dựng **Menu** màn hình con cho phần "Quản lý Truyện" **(~8% công việc)**

### 5. Khởi chạy hệ thống (Core)
- [ ] Viết sườn thực thi mã tại `main.cpp` **(~2% công việc)**
- [ ] Thiết kế và lập trình Menu chính (Main Menu) với vòng lặp vô tận điều hướng **(~5% công việc)**
- [ ] Liên kết các phím bấm/lệnh nhập vào Menu để gọi các hàm tương ứng từ UI và Repository **(~5% công việc)**

---

## 🕵️‍♂️ Nhiệm vụ của Như Ý
**Vai trò: Kỹ sư Logic Phiếu Thuê & Thống kê Thuế**

