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

### 1. Quản lý Cấu trúc & Dữ liệu Phiếu thuê (Models & Repository)
- [x] Định nghĩa `struct` thuần túy `RentalSlip` trong `models/RentalSlip.h` với các thuộc tính: `id_phieu` , `id_truyen` , `id_khach_hang` , `ngay_muon`, `ngay_tra_du_kien` , `ngay_tra_thuc_te`,`tien_coc` , `tong_tien`, `trang_thai`(0: Đang thuê, 1: Đã trả, 2: Mất/Hỏng, 3: Quá hạn) **(~5% công việc)**
- [x] Khai báo nguyên mẫu hàm trong `repository/RentalRepo.h`: `void save_rental_slip(const RentalSlip& slip);` và `void read_all_rental_slips();` **(~2% công việc)**
- [ ] Định nghĩa hàm `save_rental_slip` tại `repository/RentalRepo.cpp`: Nhận đối tượng `RentalSlip`, thực thi mở file `rentals.dat` mode `ab` (hoặc cấu hình `ios::app | ios::binary` cho fstream nếu thích) và lưu cấu trúc xuống đĩa **(~4% công việc)**
- [ ] Định nghĩa hàm `read_all_rental_slips` tại `repository/RentalRepo.cpp`: Mở file `rentals.dat` mode `rb` (hoặc `ios::in | ios::binary`), dùng `fread` trong vòng lặp `while` đọc đẩy toàn bộ lịch sử thuê vào mảng để hiển thị UI **(~4% công việc)**
- [ ] Bổ sung cơ chế ghi đè `void update_rental_status(...)`: Mở `rentals.dat` bằng `fstream` với chế độ `ios::in | ios::out | ios::binary`. Tìm đúng vị trí ID phiếu, dùng `seekp()` nhảy đến đó và ghi đè `RentalSlip` đã cập nhật (có ngày trả, tổng tiền, trạng thái mới) tại chỗ **(~5% công việc)**
- [ ] Bổ sung cơ chế ID tự tăng `int get_next_rental_id()`: Mở file `rentals.dat`, dùng `seekg(0, ios::end)` nhảy cuối file để đọc bản ghi RentalSlip cuối cùng. Lấy `id_phieu` đó cộng thêm 1. (Nếu file trống, return 1). Giảm xung đột với file metadata của A. **(~5% công việc)**

### 2. Xử lý Nghiệp vụ Logic (Services)
- [ ] Khởi tạo nguyên mẫu `services/RentalService.h` và thư viện tự tạo hỗ trợ tính ngày **(~2% công việc)**
- [ ] Viết hàm tính ngày `long date_to_days(Date d)`: Nhận vào struct thuần túy `Date {int day, month, year}` (không dùng ctime) và quy đổi ra tổng số ngày tính từ gốc năm 0 để tiện thực hiện phép trừ thời gian **(~5% công việc)**
- [ ] Viết hàm `void process_new_rental(...)`: Xử lý mượn. Lấy `gia_bia` truyện từ bên A. Khởi tạo phiếu có `trang_thai = 0`, thiết lập thu `tien_coc = 100% gia_bia`. `ngay_tra_thuc_te = {0,0,0}` và `tong_tien = 0` sinh ID và chuyển cho hàm lưu file **(~10% công việc)**
- [ ] Viết hàm `void process_return_comic(...)`: Xử lý trả. Lấy ngày hôm nay cập nhật vào `ngay_tra_thuc_te`. Cập nhật `trang_thai = 1` (Đã trả) hoặc `2` (Mất/hỏng - Tịch thu cọc) **(~10% công việc)**
- [ ] Viết hàm `void compute_payment_bill(...)` lúc trả: Gọi `date_to_days` lấy `so_ngay_thue = ngay_tra_thuc_te - ngay_muon`. Tính `tong_tien = so_ngay_thue * (10% gia_bia)`. Cộng thêm phí phạt nếu lấy `ngay_tra_thuc_te > ngay_tra_du_kien`. Cuối cùng gọi ngược lại hàm ghi đè bằng `seekp()` **(~8% công việc)**

### 3. Thống kê & Báo cáo (Statistics)
- [ ] Khởi tạo hàm `double compute_daily_revenue();` ở tầng logic: Đọc luồng dữ liệu file `rentals.dat`. Lọc những phiếu có ngày được quy định trùng với ngày hôm nay hoặc tháng này. Khấu trừ `tien_coc` tuỳ vào `trang_thai` (VD: trạng thái `2` thì thu luôn cọc) và cộng dồn lại vào tổng doanh thu **(~10% công việc)**
- [ ] Viết hàm `int count_rented_comics();`: Lặp qua file `rentals.dat`, đếm tổng số các `RentalSlip` có thuộc tính `trang_thai == 0` (đương nhiên hoặc `3` nếu bổ sung quá hạn) và hiển thị lên màn thống kê UI **(~5% công việc)**
- [ ] Viết hàm soát xét `void find_overdue_slips();`: Lập danh sách mảng các `RentalSlip` đang mượn (`trang_thai == 0`) mà check điều kiện `date_to_days(today) > date_to_days(ngay_tra_du_kien)`. Cảnh báo báo đỏ trực tiếp tại giao diện. **(~5% công việc)**

### 4. Giao diện (UI)
- [ ] Khai báo `void render_rental_menu();`: Sử dụng vòng lặp vô tận, thư viện **FTXUI**, bắt event bàn phím để xuất ra các tuỳ chọn dạng `Menu` (1. Cho thuê, 2. Trả sách, 3. Thống kê Phiếu, 4. Thoát) **(~10% công việc)**
- [ ] Khai báo `void render_revenue_table(...)`: Ứng dụng Component (Table) của **FTXUI** để đổ khung bảng Terminal hiển thị bố cục doanh thu theo dòng cột. Set thuộc tính style `.color(Color::Red)` cho sách quá hạn và mất hỏng nhằm sinh cảnh báo **(~10% công việc)**
