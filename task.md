# Danh sách nhiệm vụ dự án PBL1
*(⚠️ Tuân thủ nghiêm ngặt tiêu chuẩn `.antigravity/rules/rule.md`: Lập trình thủ tục - Không dùng Class/OOP, biến & hàm ở dạng `snake_case`, toàn bộ giao diện dùng thư viện **FTXUI**)*

## 👨‍💻 Nhiệm vụ của Khiêm
**Vai trò: Kỹ sư Dữ liệu Truyện & Nền tảng Core**

### 1. Thiết kế Cấu trúc (Models)
- [x] Thiết lập `struct` thuần túy trong `models/Comic.h` **(~3% công việc)**
- [x] Định nghĩa các thuộc tính của truyện (`id`, `comic_name`, `price`, `quantity`, `is_deleted`) **(~3% công việc)**
- [x] Thiết lập `struct` ngày tháng trong `models/Date.h` **(~2% công việc)**
- [x] Thiết lập `struct` khách hàng trong `models/Customer.h`: `id`, `name`, `phone`, `is_deleted` **(~3% công việc)**

### 2. Tiện ích (Utils)
- [x] Khởi tạo và viết tiện ích `utils/InputHandler` (Validate chống lỗi nhập chữ, chống trôi lệnh...) **(~8% công việc)**
- [x] Khởi tạo và viết tiện ích `utils/StringUtils.h` (Cắt khoảng trắng, Format tiền tệ...) **(~7% công việc)**
- [x] Viết hàm kiểm tra **chuỗi rỗng** và **số âm** phục vụ Validation chung **(~3% công việc)**
- [x] Viết hàm kiểm tra **tính hợp lệ của ngày tháng** (vd: chặn 31/02) **(~2% công việc)**

### 3. Xử lý File nhị phân (Repository)
- [x] Khởi tạo khung file `repository/ComicRepo.cpp` **(~2% công việc)**
- [x] Viết hàm **Đọc toàn bộ danh sách Truyện** **(~5% công việc)**
- [x] Viết hàm đọc/ghi `metadata.dat` để lấy ID tự động tăng **(~8% công việc)**
- [x] Viết hàm **Thêm mới truyện** và cấu hình file `comics.dat` **(~10% công việc)**
- [x] Viết hàm **Sửa thông tin truyện** **(~8% công việc)**
- [x] Viết hàm **Xóa truyện** (Logic Soft Delete) **(~2% công việc)**
- [x] Viết hàm **Tìm kiếm Truyện** **(~5% công việc)**
- [x] Khởi tạo `repository/CustomerRepo.cpp` và hàm lấy ID khách tự tăng **(~3% công việc)**
- [x] Viết các hàm **Lưu/Đọc/Sửa/Xóa (Soft Delete)** dữ liệu khách hàng **(~7% công việc)**
- [x] Viết hàm `is_comic_duplicate(...)` kiểm tra bộ đôi **Tên + Tác giả** **(~3% công việc)**
- [x] Viết hàm `is_customer_duplicate(...)` kiểm tra trùng **Số điện thoại** **(~2% công việc)**

### 4. Giao diện (UI)
- [x] Khởi tạo tính năng UI bằng thư viện **FTXUI** **(~5% công việc)**
- [x] Dùng Component `Table` in ấn màn hình theo chuẩn **(~12% công việc)**
- [x] Dùng Component dựng **Menu** quản lý truyện **(~8% công việc)**
- [x] Dựng **Menu** quản lý khách hàng bằng FTXUI **(~4% công việc)**
- [x] Hiển thị **Bảng danh sách khách hàng** dùng component Table **(~4% công việc)**
- [x] Xây dựng **Màn hình nhập liệu** và xử lý thêm/sửa khách hàng trong `CustomerUI.cpp` **(~7% công việc)**

### 5. Khởi chạy hệ thống (Core)
- [x] Viết sườn thực thi mã tại `main.cpp` **(~2% công việc)**
- [x] Thiết kế và lập trình Menu chính (Main Menu) với vòng lặp vô tận điều hướng **(~5% công việc)**
- [x] Liên kết các phím bấm/lệnh nhập vào Menu để gọi các hàm tương ứng từ UI và Repository **(~5% công việc)**

---

## 🕵️‍♂️ Nhiệm vụ của Như Ý
**Vai trò: Kỹ sư Logic Phiếu Thuê & Thống kê Thuế**

### 1. Quản lý Cấu trúc & Dữ liệu Phiếu thuê (Models & Repository)
- [x] Định nghĩa `struct` thuần túy `RentalSlip` trong `models/RentalSlip.h` với các thuộc tính: `id_phieu` , `id_truyen` , `id_khach_hang` , `ngay_muon`, `ngay_tra_du_kien` , `ngay_tra_thuc_te`,`tien_coc` , `tong_tien`, `trang_thai`(0: Đang thuê, 1: Đã trả, 2: Mất/Hỏng, 3: Quá hạn) **(~5% công việc)**
- [x] Khai báo nguyên mẫu hàm trong `repository/RentalRepo.h`: `void save_rental_slip(const RentalSlip& slip);` và `void read_all_rental_slips();` **(~2% công việc)**
- [x] Định nghĩa hàm `save_rental_slip` tại `repository/RentalRepo.cpp`: Nhận đối tượng `RentalSlip`, thực thi mở file `rentals.dat` mode `ab` (hoặc cấu hình `ios::app | ios::binary` cho fstream nếu thích) và lưu cấu trúc xuống đĩa **(~4% công việc)**
- [x] Định nghĩa hàm `read_all_rental_slips` tại `repository/RentalRepo.cpp`: Mở file `rentals.dat` mode `rb` (hoặc `ios::in | ios::binary`), dùng `fread` trong vòng lặp `while` đọc đẩy toàn bộ lịch sử thuê vào mảng để hiển thị UI **(~4% công việc)**
- [x] Bổ sung cơ chế ghi đè `void update_rental_status(...)`: Mở `rentals.dat` bằng `fstream` với chế độ `ios::in | ios::out | ios::binary`. Tìm đúng vị trí ID phiếu, dùng `seekp()` nhảy đến đó và ghi đè `RentalSlip` đã cập nhật (có ngày trả, tổng tiền, trạng thái mới) tại chỗ **(~5% công việc)**
- [x] Bổ sung cơ chế ID tự tăng `int get_next_rental_id()`: Mở file `rentals.dat`, dùng `seekg(0, ios::end)` nhảy cuối file để đọc bản ghi RentalSlip cuối cùng. Lấy `id_phieu` đó cộng thêm 1. (Nếu file trống, return 1). Giảm xung đột với file metadata của A. **(~5% công việc)**

### 2. Xử lý Nghiệp vụ Logic (Services)
- [x] Khởi tạo nguyên mẫu `services/RentalService.h` và thư viện tự tạo hỗ trợ tính ngày **(~2% công việc)**
- [x] Viết hàm tính ngày `long date_to_days(Date d)`: Nhận vào struct thuần túy `Date {int day, month, year}` (không dùng ctime) và quy đổi ra tổng số ngày tính từ gốc năm 0 để tiện thực hiện phép trừ thời gian **(~5% công việc)**
- [x] Viết hàm `void process_new_rental(...)`: Xử lý mượn. Bắt tham chiếu `gia_bia` truyện từ phân hệ Truyện (của Khiêm). Khởi tạo phiếu có `trang_thai = 0`, thiết lập thu `tien_coc = 100% gia_bia`. `ngay_tra_thuc_te = {0,0,0}` và `tong_tien = 0` sinh ID và chuyển cho hàm lưu file **(~10% công việc)**
- [x] Viết hàm `void process_return_comic(...)`: Xử lý trả. Lấy ngày hôm nay cập nhật vào `ngay_tra_thuc_te`. Cập nhật `trang_thai = 1` (Đã trả) hoặc `2` (Mất/hỏng - Tịch thu cọc) **(~10% công việc)**
- [x] Viết hàm `void compute_payment_bill(...)` lúc trả: Gọi `date_to_days` lấy `so_ngay_thue = ngay_tra_thuc_te - ngay_muon`. Tính `tong_tien = so_ngay_thue * (10% gia_bia)`. Cộng thêm phí phạt nếu lấy `ngay_tra_thuc_te > ngay_tra_du_kien`. Cuối cùng gọi ngược lại hàm ghi đè bằng `seekp()` **(~8% công việc)**
- [x] Tích hợp kiểm tra **ID truyện có tồn tại** trước khi mượn **(~3% công việc)**
- [x] Tích hợp kiểm tra **số lượng truyện > 0** **(~3% công việc)**
- [x] Tích hợp kiểm tra **Khách hàng** có tồn tại (Kiểm tra qua Số điện thoại) **(~3% công việc)**
- [x] Tích hợp kiểm tra **trùng phiếu thuê** hoạt động **(~3% công việc)**
- [x] Tích hợp kiểm tra **logic ngày trả >= ngày mượn** **(~4% công việc)**

### 3. Thống kê & Báo cáo (Statistics)
- [x] Khởi tạo hàm `double compute_daily_revenue();` ở tầng logic: Đọc luồng dữ liệu file `rentals.dat`. Lọc những phiếu có ngày được quy định trùng với ngày hôm nay hoặc tháng này. Khấu trừ `tien_coc` tuỳ vào `trang_thai` (VD: trạng thái `2` thì thu luôn cọc) và cộng dồn lại vào tổng doanh thu **(~10% công việc)**
- [x] Viết hàm `int count_rented_comics();`: Lặp qua file `rentals.dat`, đếm tổng số các `RentalSlip` có thuộc tính `trang_thai == 0` (đương nhiên hoặc `3` nếu bổ sung quá hạn) và trả về số lượng để hiển thị lên màn thống kê UI **(~5% công việc)**
- [x] Viết hàm soát xét `void find_overdue_slips(...)`: Lập danh sách mảng các `RentalSlip` đang mượn (`trang_thai == 0`) mà thoả điều kiện `date_to_days(today) > date_to_days(ngay_tra_du_kien)`. Hàm này sẽ trả về mảng/danh sách này để tầng giao diện (UI) nhận dữ liệu chứ không in trực tiếp. **(~5% công việc)**

### 4. Giao diện (UI)
- [x] Khai báo `void render_rental_menu();`: Sử dụng vòng lặp vô tận, thư viện **FTXUI**, bắt event bàn phím để xuất ra các tuỳ chọn dạng `Menu` (1. Cho thuê, 2. Trả sách, 3. Thống kê Phiếu, 4. Thoát) **(~5% công việc)**
- [x] Xây dựng màn hình **Cho thuê Truyện**: Ứng dụng chức năng `Input` của **FTXUI** để nhập ID Truyện, ID Khách hàng, sau đó gọi hàm `process_new_rental` từ tầng Service để xử lý **(~5% công việc)**
- [x] Xây dựng màn hình **Trả Truyện & Thanh toán**: Dùng chức năng `Input` nhập ID Phiếu cần xử lý trả, gọi logic tính tiền tương ứng và hiển thị Bill tổng hợp ra Terminal **(~5% công việc)**
- [x] Khai báo `void render_revenue_table(...)`: Ứng dụng Component (Table) của **FTXUI** để đổ danh sách Thống kê & Phiếu quá hạn hiển thị theo dòng cột. Set thuộc tính `.color(Color::Red)` cho sách quá hạn và mất hỏng nhằm sinh cảnh báo **(~5% công việc)**
