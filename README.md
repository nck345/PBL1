# PBL1

## Cấu trúc thư mục dự án

```text
PBL1/
│
├── .vscode/                # Cấu hình biên dịch và chạy cho VS Code
│   ├── tasks.json          # Cấu hình biên dịch (Ctrl+Shift+B)
│   └── launch.json         # Cấu hình chạy/debug (F5)
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

### Bước 2: Cài đặt Extension hỗ trợ trong VS Code
Mở VS Code lên, bấm tổ hợp phím `Ctrl + Shift + X` (Mở chợ thủ thuật Extensions) và tìm cài đặt công cụ sau:
* **C/C++** (Của nhà phát hành Microsoft, icon màu xanh/trắng)

### Bước 3: Build & Chạy chương trình
Tất cả những phần phức tạp như "Build file ra thư mục `bin/`" hay "Gom toàn bộ project để compile" đều đã được **cấu hình sẵn**. Công việc hàng ngày của bạn chỉ là:
1. Viết code.
2. Lưu file hiện tại (Ctrl + S).
3. Nhấn phím **`F5`** là chương trình sẽ tự động Cập nhật file biên dịch + Mở màn hình Debug + Chạy mượt mà!