.

🛡️ PFM Module – Persistence & File-system Monitoring
1. Tổng quan

PFM (Persistence & File-system Monitoring) là một module thuộc dự án giám sát hành vi hệ thống trên Windows, được phát triển với mục tiêu phát hiện sớm các cơ chế persistence phổ biến mà malware thường sử dụng để tự động khởi chạy khi hệ thống bật lên.

Module này được xây dựng ở user-mode, đảm bảo:
- An toàn tuyệt đối
-Không can thiệp, không chỉnh sửa hệ thống
- Chỉ giám sát và cảnh báo

2. Mục tiêu của Module PFM

PFM tập trung vào 2 kỹ thuật persistence phổ biến nhất:
- Registry Run Key
- Startup Folder

Thông qua đó, module giúp:
- Phát hiện file hoặc registry value mới được thêm
- Phát hiện sự thay đổi bất thường
- Hỗ trợ điều tra hành vi malware / unwanted software

3. Các chức năng chính
3.1. Giám sát Registry Run Key
Theo dõi key:

HKCU\Software\Microsoft\Windows\CurrentVersion\Run

Thuật toán sử dụng: Snapshot – Diff

Bước 1: Baseline

- Quét toàn bộ value trong key Run

- Lưu vào std::map<valueName, valueData>

Bước 2: Wait

- Gọi RegNotifyChangeKeyValue

- Chờ sự kiện thay đổi

Bước 3: Diff

- Quét lại toàn bộ key Run
- So sánh với baseline cũ

!!! Cảnh báo khi:

- Value mới được thêm
- Value cũ bị chỉnh sửa
- Value bị xóa

3.2. Giám sát Startup Folder

Theo dõi thư mục:

%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup

API sử dụng:

ReadDirectoryChangesW

Các sự kiện giám sát:
- File được thêm
- File bị xóa
- File bị chỉnh sửa
  
Cờ theo dõi:
- FILE_NOTIFY_CHANGE_FILE_NAME
- FILE_NOTIFY_CHANGE_SIZE


Lưu ý: Module xử lý chính xác buffer chứa nhiều FILE_NOTIFY_INFORMATION nối tiếp nhau.

4. Cấu trúc thư mục (PFM Module)
 ```
PFM_Monitoring/
│
├── Header Files/
│   ├── Common.h
│   ├── Logger.h
│   ├── RegistryMonitor.h
│   └── StartupMonitor.h
│
├── Source Files/
│   ├── Logger.cpp
│   ├── RegistryMonitor.cpp
│   ├── StartupMonitor.cpp
│   └── PFM_Monitoring.cpp   // main
│
├── logs/
│   └── registry.log         
│
└── README_PFM.md
```
6. Logging & Output

Log được in ra console

Đồng thời ghi vào file:

logs/registry.log


Ví dụ log:

[2025-12-12T16:59:30] [WARN] [ALERT] Startup File Added: notepad.lnk
[2025-12-12T17:00:10] [WARN] [ALERT] Registry Item Modified: OneDrive

6. Độ an toàn

✔ Không ghi registry
✔ Không xóa file
✔ Không inject / hook
✔ Không yêu cầu quyền admin

➡️ An toàn tuyệt đối khi chạy trên máy thật

7. Công nghệ sử dụng

- Ngôn ngữ: C++ (Win32 API)
- Môi trường: Windows User-mode
- API chính:
    - RegEnumValueW
    - RegNotifyChangeKeyValue
    - ReadDirectoryChangesW
    - CreateEvent / WaitForSingleObject
