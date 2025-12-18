# 📁 Cấu trúc Project WindowsBehaviorMonitor

## 🎯 Tổng quan
Project đã được tổ chức lại thành cấu trúc module rõ ràng, dễ bảo trì và phát triển.

## 📂 Cấu trúc thư mục mới

```
WindowsBehaviorMonitor/
│
├── WindowsBehaviorMonitor.cpp    # Main program với giao diện menu
├── WindowsBehaviorMonitor.sln    # Visual Studio Solution
├── WindowsBehaviorMonitor.vcxproj
│
├── PMM/                           # Process & Memory Monitoring Module
│   ├── ProcessManager.h           # Header cho PMM
│   └── ProcessManager.cpp         # Implementation PMM
│
├── PFM/                           # Persistence & File-system Monitoring
│   ├── RegistryMonitor.h          # Monitor HKCU\Run
│   ├── RegistryMonitor.cpp
│   ├── StartupMonitor.h           # Monitor Startup folder
│   └── StartupMonitor.cpp
│
├── NMM/                           # Network Monitoring Module
│   ├── NetworkMonitor.h           # Header cho NMM
│   └── NetworkMonitor.cpp         # Monitor TCP connections
│
├── Common/                        # Shared utilities
│   ├── Logger.h                   # Logging system
│   ├── Logger.cpp
│   └── EventStruct.h              # Event data structures
│
├── MockMalwareSim/                # Test tool cho PMM
│   └── MockMalwareSim.cpp
│
├── PFM_Monitoring/                # [OLD] Legacy folder (có thể xóa)
├── NMM_Monitoring/                # [OLD] Legacy folder (có thể xóa)
│
└── README.md                      # Project documentation
```

## 🎨 Giao diện mới

### Main Menu
```
  ╔═══════════════════════════════════════════════════════════════════╗
  ║        █     █ █ █   █ ███  ███  █   █ ███                       ║
  ║        █  █  █ █ ██  █ █  █ █  █ █   █ █                         ║
  ║        ███████ █ █ █ █ █  █ █  █ █ █ █ ███                       ║
  ║        █     █ █ █  ██ █  █ █  █ █ █ █ █                         ║
  ║        █     █ █ █   █ ███  ███  ███ █ ███                       ║
  ║              BEHAVIOR MONITOR SYSTEM v1.0                         ║
  ╚═══════════════════════════════════════════════════════════════════╝

  【 MODULE SELECTION 】

    [1] PMM - Process & Memory Monitoring
    [2] PFM - Persistence & File-system Monitoring
    [3] NMM - Network Monitoring Module
    [0] Exit Program
```

### Tính năng giao diện:
- ✅ ASCII Art Banner đẹp mắt
- ✅ Màu sắc phân biệt từng module (Cyan, Yellow, Green, Magenta)
- ✅ Status bar hiển thị module nào đang chạy
- ✅ Icons đẹp (✓, ✗, ⚠, ►, •)
- ✅ Menu phân cấp rõ ràng cho từng module
- ✅ Hỗ trợ real-time monitoring với threading

## 🧩 Mô tả các Module

### 1. PMM (Process & Memory Monitoring) - Phùng Đức Anh
**Location:** `PMM/ProcessManager.h`, `PMM/ProcessManager.cpp`

**Chức năng:**
- List all running processes
- Scan single process memory
- Enable SeDebugPrivilege
- Scan all processes for suspicious memory
- Test memory scanner với MockMalwareSim

**Menu PMM:**
```
[1] List All Running Processes
[2] Scan Single Process Memory
[3] Enable SeDebugPrivilege (Administrator)
[4] Scan All Processes for Suspicious Memory
[5] Test Memory Scanner (MockMalwareSim)
[0] Back to Main Menu
```

### 2. PFM (Persistence & File-system Monitoring) - Nguyễn Thị Diệu
**Location:** `PFM/RegistryMonitor.h/cpp`, `PFM/StartupMonitor.h/cpp`

**Chức năng:**
- Real-time Registry monitoring (HKCU\Run)
- Real-time Startup folder monitoring
- Phát hiện thêm/xóa/sửa Registry keys
- Phát hiện thêm/xóa/sửa files trong Startup folder

**Menu PFM:**
```
[1] Start Registry & Startup Monitoring (Real-time)
[2] Stop Monitoring
[3] Show Registry Baseline (HKCU\Run)
[4] List Startup Folder Files
[0] Back to Main Menu
```

### 3. NMM (Network Monitoring Module) - Nguyễn Trí Như
**Location:** `NMM/NetworkMonitor.h`, `NMM/NetworkMonitor.cpp`

**Chức năng:**
- Real-time TCP connection monitoring
- Phát hiện kết nối mới
- Display current TCP connections

**Menu NMM:**
```
[1] Start Network Monitoring (Real-time)
[2] Stop Monitoring
[3] Display Current TCP Connections
[0] Back to Main Menu
```

### 4. Common (Shared Utilities)
**Location:** `Common/`

**Nội dung:**
- `Logger.h/cpp`: Logging system cho toàn bộ project
- `EventStruct.h`: Data structures cho events

## 🔧 Cách build và chạy

### Cập nhật Visual Studio Project
Bạn cần update file `.vcxproj` để include các file mới:

1. Mở Visual Studio 2022
2. Load `WindowsBehaviorMonitor.sln`
3. Add các files vào project:
   - PMM/ProcessManager.h, PMM/ProcessManager.cpp
   - PFM/RegistryMonitor.h, PFM/RegistryMonitor.cpp
   - PFM/StartupMonitor.h, PFM/StartupMonitor.cpp
   - NMM/NetworkMonitor.h, NMM/NetworkMonitor.cpp
   - Common/Logger.h, Common/Logger.cpp
   - Common/EventStruct.h
4. Build Solution (Ctrl+Shift+B)
5. Run (F5 hoặc Ctrl+F5)

### Hoặc dùng command line:
```powershell
# Build
msbuild WindowsBehaviorMonitor.sln /p:Configuration=Debug /p:Platform=x64

# Run
.\x64\Debug\WindowsBehaviorMonitor.exe
```

## 🎯 Cải tiến so với version cũ

### ✅ Code Organization
- ✅ Tách biệt rõ ràng 3 module theo folder
- ✅ Common utilities được tách riêng
- ✅ Dễ maintain và scale up

### ✅ User Interface
- ✅ Giao diện terminal đẹp với ASCII art
- ✅ Màu sắc phân biệt module
- ✅ Status bar real-time
- ✅ Menu phân cấp rõ ràng

### ✅ Functionality
- ✅ Real-time monitoring với threading
- ✅ Có thể chạy nhiều module cùng lúc
- ✅ Status bar hiển thị module nào đang active
- ✅ Graceful shutdown khi thoát

### ✅ Code Quality
- ✅ Thread-safe với std::atomic
- ✅ Proper cleanup khi exit
- ✅ Error handling tốt hơn
- ✅ Logging system cho toàn project

## 📝 Notes

### Cleanup Legacy Folders
Sau khi đã test kỹ, bạn có thể xóa các folder cũ:
```powershell
Remove-Item -Recurse "PFM_Monitoring"
Remove-Item -Recurse "NMM_Monitoring"
Remove-Item "WindowsBehaviorMonitor_OLD.cpp"
```

### Testing
- Test từng module riêng lẻ
- Test chạy nhiều module cùng lúc
- Test thoát gracefully khi module đang chạy
- Test với MockMalwareSim cho PMM

## 👥 Team Members
- **Phùng Đức Anh** - PMM Module
- **Nguyễn Thị Diệu** - PFM Module  
- **Nguyễn Trí Như** - NMM Module

## 📅 Version History
- **v1.0** (Current) - Integrated all modules with beautiful terminal UI
- **v0.x** - Individual module development

---
**Last Updated:** December 2025
