# 📚 ỨNG DỤNG QUẢN LÝ CHO THUÊ TRUYỆN TRANH (PBL1)

Ứng dụng Quản lý Cho thuê Truyện tranh là dự án đồ án PBL1 được phát triển bằng ngôn ngữ **C++**, kết hợp thư viện giao diện terminal hiện đại **FTXUI** để mang lại trải nghiệm TUI (Terminal User Interface) trực quan, sinh động. Hệ thống lưu trữ dữ liệu an toàn dưới dạng các file nhị phân (`.dat`) và triển khai các thuật toán tìm kiếm, sắp xếp và kiểm tra dữ liệu đầu vào tùy biến.

---

## ✨ Tính năng chính

### 1. Quản lý Truyện tranh (Comics)
* Thêm mới truyện tranh (tự sinh ID tăng dần).
* Sửa thông tin truyện (tên truyện, tác giả, giá thuê, giá trị cọc, số lượng tồn kho).
* Xóa truyện tranh (áp dụng phương pháp xóa mềm - soft delete thông qua thuộc tính `is_deleted`).
* Tìm kiếm truyện tranh nâng cao (theo tên, mã truyện) sử dụng tìm kiếm nhị phân/tuyến tính.
* Sắp xếp danh sách truyện tranh theo nhiều tiêu chí (ID, giá thuê, tên) sử dụng thuật toán tối ưu.

### 2. Quản lý Khách hàng (Customers)
* Lưu trữ và hiển thị danh sách khách hàng thân thiết.
* Thêm mới, cập nhật thông tin và xóa khách hàng.
* Tra cứu thông tin khách hàng nhanh chóng qua số điện thoại hoặc mã khách hàng.

### 3. Quản lý Phiếu thuê (Rental Slips)
* Tạo phiếu thuê truyện ghi nhận chi tiết: Khách hàng, các truyện đã thuê, ngày thuê, hạn trả, và tiền cọc.
* Nhận trả truyện: Tính toán tự động số ngày thuê thực tế, đối chiếu hạn trả và tự động tính phí phạt nếu trả quá hạn hoặc làm mất sách.
* Cập nhật trạng thái phiếu thuê trực tiếp trên file nhị phân.

### 4. Thống kê & Báo cáo (Statistics)
* Thống kê doanh thu theo ngày và theo tháng.
* Đếm số lượng đầu truyện đang được cho thuê.
* Thống kê tổng số truyện bị mất và tổng thiệt hại để quản trị kho sách hiệu quả.

---

## 📁 Cấu trúc thư mục dự án

```text
PBL1/
├── bin/                        # Chứa file thực thi sau khi biên dịch (.exe)
│   └── main.exe
├── obj/                        # Chứa các file đối tượng biên dịch trung gian (.o)
├── data/                       # Thư mục lưu trữ cơ sở dữ liệu nhị phân (.dat)
│   ├── comics.dat              # Dữ liệu các cuốn truyện tranh
│   ├── customers.dat           # Dữ liệu danh sách khách hàng
│   ├── rentals.dat             # Dữ liệu lịch sử phiếu thuê truyện
│   ├── metadata.dat            # Lưu trữ ID tự tăng tiếp theo của truyện tranh
│   └── customer_id.dat         # Lưu trữ ID tự tăng tiếp theo của khách hàng
├── include/                    # Khai báo cấu trúc dữ liệu và mẫu hàm (.h)
│   ├── models/                 # Các struct đại diện thực thể (Comic, Customer, Date, RentalSlip)
│   ├── repository/             # Giao tiếp đọc/ghi tệp nhị phân (ComicRepo, CustomerRepo, RentalRepo)
│   ├── services/               # Logic tính tiền, phạt quá hạn và điều phối nghiệp vụ (RentalService)
│   ├── ui/                     # Giao diện chính và hệ thống màu sắc (UITheme, ComicUI,...)
│   │   ├── comic/              # Giao diện TUI quản lý truyện tranh (thêm, sửa, xem, xóa)
│   │   ├── customer/           # Giao diện TUI quản lý khách hàng (thêm, sửa, xem, xóa)
│   │   ├── rental/             # Giao diện TUI quản lý phiếu thuê (tạo phiếu, trả truyện, xem)
│   │   └── statistics/         # Giao diện TUI thống kê doanh thu và báo cáo kho
│   └── utils/                  # Thuật toán bổ trợ (InputHandler, ValidationUtils, SortUtils, SearchUtils)
├── src/                        # Triển khai chi tiết mã nguồn (.cpp)
│   ├── repository/             # Triển khai thao tác đọc/ghi file nhị phân tại chỗ (seekp, seekg)
│   ├── services/               # Triển khai các tính năng tính toán nghiệp vụ
│   ├── ui/                     # Triển khai thiết kế giao diện FTXUI tương tác
│   │   ├── comic/
│   │   ├── customer/
│   │   ├── rental/
│   │   └── statistics/
│   ├── utils/                  # Triển khai thuật toán tìm kiếm nhị phân, QuickSort và bắt lỗi nhập liệu
│   └── main.cpp                # Hàm main khởi chạy vòng lặp menu giao diện chính
├── Makefile                    # Kịch bản tự động biên dịch cho MSYS2 / MinGW G++
└── README.md                   # Tài liệu hướng dẫn dự án
```

---

## 🛠️ Hướng dẫn cài đặt & Khởi chạy

### 1. Yêu cầu hệ thống (Prerequisites)
Để biên dịch và chạy dự án thành công trên Windows, bạn cần cài đặt:
1. **MSYS2 (UCRT64)**: Bộ môi trường phát triển C++.
2. **Trình biên dịch G++** & công cụ **Make** được cài đặt thông qua MSYS2.
3. **Thư viện FTXUI**: Được tích hợp sẵn trong môi trường biên dịch của bạn (liên kết qua các cờ biên dịch `-lftxui-component -lftxui-dom -lftxui-screen`).

### 2. Các lệnh biên dịch bằng Makefile

Mở terminal tại thư mục gốc của dự án và chạy các lệnh sau:

* **Biên dịch toàn bộ dự án**:
  ```bash
  make
  ```
  *Lưu ý: Lệnh này sẽ tự động quét toàn bộ mã nguồn trong `src/`, dịch thành các file đối tượng trong thư mục `obj/` và xuất ra file chạy `bin/main.exe`.*

* **Khởi chạy ứng dụng**:
  Chạy trực tiếp file đã biên dịch:
  ```bash
  ./bin/main.exe
  ```

* **Dọn dẹp các tệp tin build**:
  Để xóa bỏ thư mục lưu trữ file `.o` trung gian (`obj/`) và file thực thi (`bin/main.exe`) khi muốn build lại từ đầu:
  ```bash
  make clean
  ```

### 3. Biên dịch & Chạy nhanh qua VS Code (Khuyên dùng)
Dự án đã được cấu hình sẵn trong thư mục `.vscode/` để tối ưu hóa trải nghiệm trên Visual Studio Code:
* **Nhấn phím `F5`** (hoặc chọn menu **Run -> Start Debugging**): Hệ thống sẽ tự động chạy tác vụ biên dịch dự án (`mingw32-make.exe`), sau đó khởi chạy ứng dụng trực tiếp trên một cửa sổ Terminal độc lập.
* **Gỡ lỗi (Debugging)**: Bạn có thể đặt các breakpoint trực tiếp trong các tệp `.cpp` để gỡ lỗi chương trình bằng trình gỡ lỗi GDB đã cấu hình sẵn.

### 4. Lưu ý về tệp dữ liệu (`data/`)
* Thư mục `data/` chứa các tệp cơ sở dữ liệu `.dat`. Để tránh xung đột dữ liệu giữa các thành viên khi đẩy code lên GitHub, các tệp nhị phân này đã được cấu hình bỏ qua trong `.gitignore`.
* Chương trình có cơ chế tự động tạo mới các tệp dữ liệu trắng nếu chưa tìm thấy tệp tin tương ứng trong thư mục `data/`, giúp ứng dụng luôn sẵn sàng chạy ngay sau khi build.