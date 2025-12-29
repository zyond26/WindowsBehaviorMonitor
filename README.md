# WindowsBehaviorMonitor 
**A user-mode endpoint monitoring tool for Windows**

Đây là một công cụ **giám sát hành vi hệ thống ở user-mode trên Windows**, được xây dựng như một dự án thực tập nhằm mô phỏng các thành phần cơ bản của một **EDR (Endpoint Detection & Response)**.

## ✨ Tính năng mới (v1.0)
- ✅ **Giao diện Terminal đẹp** với ASCII art và màu sắc
- ✅ **Real-time Monitoring** với multi-threading
- ✅ **Menu phân cấp** dễ sử dụng
- ✅ **Status Bar** hiển thị trạng thái các module
- ✅ **Code được tổ chức** theo module rõ ràng

Dự án tập trung vào việc **phát hiện sớm các hành vi bất thường** liên quan đến:
- 🔍 **PMM** - Process & Memory Monitoring
- 📁 **PFM** - Persistence & File-system Monitoring  
- 🌐 **NMM** - Network Monitoring Module

## 🎯 Mục tiêu dự án

- Xây dựng nền tảng giám sát hệ thống Windows bằng **C/C++ và Windows API**
- Hiểu rõ cách malware hoạt động ở user-mode
- Áp dụng các kỹ thuật giám sát thực tế nhưng **an toàn, không phá hệ thống**
- Rèn luyện tư duy chia module và làm việc nhóm theo Sprint

## 🧩 Kiến trúc tổng thể

WindowsBehaviorMonitor được tổ chức thành cấu trúc module rõ ràng:

```text
WindowsBehaviorMonitor/
│
├── WindowsBehaviorMonitor.cpp    # Main program với giao diện menu đẹp
│
├── PMM/                           # Process & Memory Monitoring
│   ├── ProcessManager.h
│   └── ProcessManager.cpp
│
├── PFM/                           # Persistence & File-system Monitoring
│   ├── RegistryMonitor.h/.cpp
│   └── StartupMonitor.h/.cpp
│
├── NMM/                           # Network Monitoring Module
│   ├── NetworkMonitor.h
│   └── NetworkMonitor.cpp
│
└── Common/                        # Shared utilities
    ├── Logger.h/.cpp
    └── EventStruct.h
```

### 🎨 Giao diện Terminal
- ✨ ASCII Art Banner đẹp mắt
- 🎨 Màu sắc phân biệt từng module
- 📊 Status bar real-time
- 📋 Menu phân cấp trực quan
## 👥 Phân công nhiệm vụ

### 🔹 Phùng Đức Anh – **PMM (Process & Memory Monitoring)**

**Chức năng chính:**
- Liệt kê toàn bộ tiến trình đang chạy
- Theo dõi PID và `CreationTime` để tránh nhầm lẫn khi PID bị tái sử dụng
- Quét không gian bộ nhớ tiến trình để phát hiện hành vi **Process Injection**

**Kỹ thuật sử dụng:**
- `CreateToolhelp32Snapshot`
- `Process32First / Process32Next`
- `OpenProcess`
- `GetProcessTimes`
- `VirtualQueryEx`

**Heuristic phát hiện:**
- Vùng nhớ:
  - `MEM_COMMIT`
  - `PAGE_EXECUTE_READWRITE`
  - `MEM_PRIVATE`

→ Cảnh báo khi phát hiện **RWX Private Memory**

### 🔹 Nguyễn Thị Diệu – **PFM (Persistence & File-system Monitoring)**

**Chức năng chính:**
- Giám sát các cơ chế persistence phổ biến của malware
- Phát hiện thay đổi Registry và Startup folder theo thời gian thực

#### 1. Registry Monitoring (HKCU Run)
- Key giám sát:
HKCU\Software\Microsoft\Windows\CurrentVersion\Run

- Thuật toán **Snapshot – Diff**
- Tạo baseline ban đầu
- Chờ sự kiện bằng `RegNotifyChangeKeyValue`
- So sánh để phát hiện:
  - New Item Added
  - Item Modified
  - Item Removed

#### 2. Startup Folder Monitoring
- Thư mục:
%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup

- Theo dõi bằng:
- `ReadDirectoryChangesW`
- Xử lý `FILE_NOTIFY_INFORMATION`
- Phát hiện:
- File được thêm
- File bị xóa
- File bị chỉnh sửa
- Rename (old/new)

**Đảm bảo:**
- Chỉ đọc (read-only)
- Không ghi, không sửa Registry hay file hệ thống
- An toàn tuyệt đối khi chạy trên máy thật

### 🔹 Nguyễn Trí Như – **NMM (Network Monitoring Module)**

**Chức năng chính:**
- Giám sát các kết nối TCP đang hoạt động
- Phát hiện các kết nối mới được tạo

**Kỹ thuật sử dụng:**
- `GetExtendedTcpTable`
- Lấy thông tin:
- Local IP / Port
- Remote IP / Port
- PID tương ứng

**Logic phát hiện:**
- So sánh bảng TCP hiện tại với snapshot trước đó
- Nếu có kết nối mới → ghi nhận sự kiện

## 🧪 Kiểm thử

- Test trên **máy thật (user-mode)**: an toàn
- Test nâng cao trong **máy ảo (VirtualBox)**
- Network: Host-Only / Internal
- Không dùng NAT hoặc Bridged
- Sử dụng các hành vi mô phỏng:
- Thêm Registry Run key
- Thêm shortcut vào Startup folder
- Tạo kết nối TCP giả lập

## ⚠️ Lưu ý an toàn

-  WindowsBehaviorMonitor **KHÔNG phải malware**
- Không inject code
- Không ghi registry
- Không chỉnh sửa file hệ thống
- Dự án mang tính **nghiên cứu – học tập – đào tạo**

## 🛠 Công nghệ sử dụng

- **C++17** với Modern C++ features
- **Windows API** (Process, Registry, Network, Threading)
- **Multi-threading** (std::thread, std::atomic)
- **Visual Studio 2022**
- **Git / GitHub**

## 🚀 Cách chạy

### Build từ Visual Studio
1. Mở `WindowsBehaviorMonitor.sln` trong Visual Studio 2022
2. Build Solution (Ctrl+Shift+B)
3. Run (F5) hoặc Run without debugging (Ctrl+F5)

### Build từ Command Line
```powershell
msbuild WindowsBehaviorMonitor.sln /p:Configuration=Debug /p:Platform=x64
.\x64\Debug\WindowsBehaviorMonitor.exe
```

### Sử dụng
Chương trình sẽ hiển thị menu:
- **[1] PMM** - Giám sát process và memory
- **[2] PFM** - Giám sát Registry và Startup folder
- **[3] NMM** - Giám sát kết nối TCP
- **[0]** Thoát

Mỗi module có sub-menu riêng với các chức năng chi tiết.

## 📄 License
+) Dự án phục vụ mục đích học tập và nghiên cứu nội bộ.
+) Dự án mang tính nghiên cứu – học tập – đào tạo

## 👤 Nhóm thực hiện
- Phùng Đức Anh – PMM
- Nguyễn Thị Diệu – PFM
- Nguyễn Trí Như – NMM
