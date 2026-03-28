# PBL1

## Cấu trúc thư mục dự án

```text
PBL1/
│
├── .vscode/                # Cấu hình biên dịch và chạy cho VS Code
│   ├── tasks.json          # Cấu hình biên dịch (Ctrl+Shift+B)
│   └── launch.json         # Cấu hình chạy/debug (F5)
│
├── .antigravity/           # Cấu hình quy tắc
│   └── rules/rule.md       # Các quy tắc phát triển dự án bắt buộc
│
├── bin/                    # Chứa file thực thi sau khi build (.exe)
│   └── main.exe            # Sản phẩm biên dịch
│   
├── data/                   # Lưu trữ các file nhị phân (.dat)
│   ├── comics.dat          # Danh mục truyện
│   ├── rentals.dat         # Danh sách phiếu thuê
│   └── metadata.dat        # Lưu các biến toàn cục (VD: ID tự tăng tiếp theo)
│
├── include/                # Khai báo (.h)
│   ├── models/             # Cấu trúc dữ liệu
│   ├── repository/         # Tương tác file
│   ├── services/           # Logic nghiệp vụ
│   ├── ui/                 # Giao diện
│   └── utils/              # Hàm tiện ích dùng chung
│       ├── StringUtils.h   # Cắt khoảng trắng, in hoa, format currency
│       └── ConsoleUtils.h  # Xóa màn hình, dừng màn hình chờ, đổi màu chữ
│
├── src/                    # Mã nguồn triển khai (.cpp)
│   ├── models/             
│   ├── repository/         
│   ├── services/           
│   ├── ui/                 
│   ├── utils/              
│   └── main.cpp            # Chức năng chính
│
└── README.md               # Mô tả dự án
```

---

## Các thư viện sử dụng trong dự án

Dự án sử dụng các thư viện và công cụ hiện đại để tối ưu hóa trải nghiệm người dùng và tốc độ phát triển:

*   **[FTXUI](https://github.com/ArthurSonzogni/FTXUI)**: Thư viện C++ mạnh mẽ để xây dựng giao diện người dùng trên Terminal (TUI). Hỗ trợ các thành phần tương tác như Menu, Button, Input, Table và các hiệu ứng đồ họa mượt mà.
*   **GNU Make (mingw32-make)**: Hệ thống quản lý biên dịch thông minh (Incremental Build). Chỉ biên dịch lại những phần code có thay đổi, giúp tiết kiệm thời gian build (dưới 1 giây cho các lần sửa đổi nhỏ).

---

## Đối với Thành viên trong nhóm (Team Setup)

Dự án này đã được cấu hình tự động hoàn toàn trên VS Code thông qua thư mục `.vscode/` (chia sẻ chung cho cả đội). Để chạy được code mà không gặp bất kỳ lỗi gì, các thành viên chỉ cần làm đúng **1 lần duy nhất** theo các bước sau:

### Bước 1: Cài đặt Trình biên dịch C++ (Nếu máy chưa có)
1. Truy cập trang chủ [MSYS2](https://www.msys2.org/) và tải file cài đặt (File `.exe` màu xanh lá).
2. Cài đặt MSYS2 với các tuỳ chọn mặc định (Thường nó sẽ cài vào ổ `C:\msys64`).
3. Sau khi cài xong, mở chương trình **MSYS2 UCRT64** (hình cửa sổ terminal gõ lệnh màu xám) từ menu Start.
4. Gõ chính xác dòng lệnh sau và nhấn **Enter**:
   ```bash
   pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-gdb
   ```
5. Nhấn `Y` để đồng ý tải và cài đặt bộ Compile (Biên dịch) và Debug (Gỡ lỗi) C++.
6. **Thêm biến môi trường (PATH):**
   * Mở Start Menu của Windows, gõ tìm kiếm `Environment Variables` (hoặc `Edit the system environment variables`).
   * Chọn nút **Environment Variables...**.
   * Ở bảng "User variables" hoặc "System variables", tìm dòng chữ `Path`, nhấn đúp chuột vào nó.
   * Nhấn **New**, sau đó dán đường dẫn này vào: `C:\msys64\ucrt64\bin`
   * Nhấn OK toàn bộ các bảng để lưu lại.

### Bước 2: Cài đặt thư viện giao diện FTXUI
1. Tại khu vực Terminal của VS Code, hãy mở cửa sổ dòng lệnh **PowerShell**.
2. Dán nguyên lệnh sau vào và nhấn **Enter** để tải thư viện FTXUI về máy:
   ```powershell
   C:\msys64\usr\bin\pacman.exe -S mingw-w64-ucrt-x86_64-ftxui
   ```
3. Nhập `Y` (hoặc `y`) và nhấn Enter nếu hệ thống hỏi xác nhận.
*(Lưu ý: Dự án đã được cấu hình sẵn để tự động liên kết với thư viện này trong `tasks.json`, bạn chỉ cần tải về là xong!)*

### Bước 3: Cài đặt Extension hỗ trợ trong VS Code
Mở VS Code lên, bấm tổ hợp phím `Ctrl + Shift + X` (Mở chợ thủ thuật Extensions) và tìm cài đặt công cụ sau:
* **C/C++** (Của nhà phát hành Microsoft, icon màu xanh/trắng)

### Bước 4: Build & Chạy chương trình
Tất cả những phần phức tạp như "Build file ra thư mục `bin/`" hay "Gom toàn bộ project để compile" đều đã được **cấu hình sẵn**. Công việc hàng ngày của bạn chỉ là:
1. Viết code.
2. Lưu file hiện tại (Ctrl + S).
3. Nhấn phím **`F5`** là chương trình sẽ tự động Cập nhật file biên dịch + Mở màn hình Debug + Chạy mượt mà!

---

## Phân công Nhiệm vụ 

### 👨‍💻 Khiêm: Kỹ sư Dữ liệu Truyện & Nền tảng Core
**Nhiệm vụ:** Xây dựng móng vững chắc cho dự án và Quản lý toàn bộ danh mục Truyện.
*   **Thiết kế Cấu trúc (Models):** Định nghĩa file `models/Comic.h` (id, tên truyện, giá, số lượng) và `models/Date.h` (Ngày tháng dùng chung).
*   **Xử lý File nhị phân (Repository):** Hoàn thiện `repository/ComicRepo.cpp` để thực hiện chức năng Thêm mới, Sửa thông tin, Xóa truyện (Soft Delete) và Tìm kiếm Truyện trên file `comics.dat`.
*   **Khởi chạy hệ thống (Core):** Xây dựng bộ Menu chính điều hướng của chương trình tại `main.cpp`.
*   **Tiện ích:** Hỗ trợ viết các hàm nhập liệu chống lỗi (Validate input) tại thư viện `utils/InputHandler`.
*   **Giao diện (UI):** Thiết kế màn hình "Quản lý Truyện", in ấn danh sách các bộ truyện dưới dạng bảng ngay ngắn, đẹp mắt.

### 🕵️‍♂️ Như Ý: Kỹ sư Logic Phiếu Thuê & Thống kê Thuế
**Nhiệm vụ:** Xử lý các nghiệp vụ (thuật toán) khó nhất của phần mềm, móc nối dữ liệu giữa Phiếu Thuê và Kho Truyện.
*   **Quản lý Phiếu thuê (Rental):** Định nghĩa `models/RentalSlip.h` và xây dựng `repository/RentalRepo.cpp` để lưu các phiếu do khách hàng thuê xuống file `rentals.dat`.
*   **Xử lý Nghiệp vụ Cho Thuê (Services):**
    *   `RentalService`: Khi khách hàng muốn thuê, Như Ý phải gọi hàm lấy truyện của Khiêm để kiểm tra truyện có còn trong kho không. Nếu còn $\rightarrow$ tạo phiếu $\rightarrow$ tự động trừ số lượng truyện trong kho.
    *   Định giá: Viết hàm tính số dư ngày mượn/ngày trả để quy ra tiền cọc và tiền thanh toán thực tế.
*   **Thống kê Báo cáo (Statistics):** Chạy vòng quét toàn bộ dữ liệu ở cả 2 file `.dat` để đếm: Hôm nay/Tháng này thu được bao nhiêu tiền? Kho bị mất mát bao nhiêu quyển, đang cho thuê ra ngoài bao nhiêu quyển?
*   **Giao diện (UI):** Xây dựng Menu "Quản lý Phiếu Thuê" và thiết kế Bảng Thống kê Doanh Thu cuối ngày cực xịn xò.