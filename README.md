# Windows Behavior Monitor - Complete System

## 📋 Tổng quan

Hệ thống phát hiện hành vi đáng ngờ trong Windows processes, đặc biệt tập trung vào việc phát hiện **RWX (Read-Write-Execute) memory regions** - một kỹ thuật phổ biến được malware sử dụng.

## 🏗️ Kiến trúc

### 1. **WindowsBehaviorMonitor** (Main EDR Scanner)
- **Chức năng chính**: Giám sát và quét processes trong hệ thống
- **File**: `WindowsBehaviorMonitor.exe`
- **Các tính năng**:
  - ✅ List tất cả running processes
  - ✅ Scan memory của process cụ thể
  - ✅ Enable SeDebugPrivilege (để quét protected processes)
  - ✅ Scan toàn bộ hệ thống
  - ✅ Test scanner với target process

### 2. **MockMalwareSim** (Testing Tool)
- **Chức năng**: Giả lập hành vi RWX memory để test scanner
- **File**: `MockMalwareSim.exe`
- **Hoạt động**:
  - Allocate 1MB memory với `PAGE_EXECUTE_READWRITE`
  - Display PID và Base Address
  - Giữ process alive để test
  - **An toàn**: Chỉ dùng cho testing

### 3. **ProcessManager** (Core Library)
- **Files**: `ProcessManager.cpp`, `ProcessManager.h`
- **Chức năng chính**:

#### `EnableSeDebugPrivilege()`
```cpp
static bool EnableSeDebugPrivilege();
```
- Kích hoạt quyền debug cho process hiện tại
- Cần thiết để quét các protected processes

#### `GetRunningProcesses()`
```cpp
ProcessMap GetRunningProcesses();
```
- Liệt kê tất cả processes đang chạy
- Trả về map: `<PID, ProcessInfo>`

#### `ScanProcessMemory(DWORD pid)`
```cpp
std::wstring ScanProcessMemory(DWORD pid);
```
- Quét memory của process theo PID
- Phát hiện RWX memory regions
- **Detection criteria**:
  - ✅ State = `MEM_COMMIT`
  - ✅ Type = `MEM_PRIVATE`
  - ✅ Protection = `PAGE_EXECUTE_READWRITE (0x40)`

#### `IsAllowlisted(processName)`
```cpp
private: bool IsAllowlisted(const std::wstring& processName);
```
- Kiểm tra process có trong allowlist không
- **Allowlist**: Chrome, Edge, Firefox, Java, Python, Node.js, etc.
- Giảm false positives từ JIT compilers

#### `TestMemoryScanner(DWORD targetPID)`
```cpp
void TestMemoryScanner(DWORD targetPID);
```
- Unit test function
- Verify scanner hoạt động đúng
- So sánh kết quả với MockMalwareSim

## 🎯 Workflow Testing

```
┌─────────────────────┐
│  MockMalwareSim     │
│  - Allocate RWX     │
│  - Show PID/Addr    │
│  - Stay Running     │
└─────────┬───────────┘
          │
          │ PID: 12345
          │ Addr: 0x...
          ↓
┌─────────────────────┐
│ Scanner (as Admin)  │
│  1. Enable Debug    │
│  2. Test Scanner    │
│  3. Input PID       │
└─────────┬───────────┘
          │
          ↓
    ┌─────────┐
    │ Result  │
    │ ✅ PASS │
    │ ❌ FAIL │
    └─────────┘
```


## Manual Testing (Chi tiết trong TESTING_GUIDE.md)

**Terminal 1 - Target:**
```powershell
cd "F:\Github\WindowsBehaviorMonitor\x64\Debug"
.\MockMalwareSim.exe
# Ghi nhớ PID và Base Address
```

**Terminal 2 - Scanner (as Admin):**
```powershell
cd "F:\Github\WindowsBehaviorMonitor\x64\Debug"
.\WindowsBehaviorMonitor.exe
# Option 3: Enable SeDebugPrivilege
# Option 5: Test Memory Scanner
# Nhập PID từ Terminal 1
```

## 📊 Kết quả mong đợi

### ✅ Test Passed
```
TEST PASSED: RWX Injection Detected!

Detection Details:
Suspicious region: Base=0x1E093D70000, Size=0x100000

Verification Instructions:
Compare Base Address with MockMalwareSim
```

### ❌ Test Failed
```
TEST FAILED: No RWX regions detected!
Possible reasons:
  - Process is allowlisted
  - Insufficient permissions
  - Invalid PID
```

## 🛡️ Detection Logic

### Thuật toán phát hiện:
1. Open target process với `PROCESS_QUERY_INFORMATION | PROCESS_VM_READ`
2. Loop qua toàn bộ virtual address space với `VirtualQueryEx`
3. Check mỗi memory region:
   ```cpp
   if (mbi.State == MEM_COMMIT &&
       mbi.Type == MEM_PRIVATE &&
       mbi.Protect == PAGE_EXECUTE_READWRITE)
   {
       // ⚠️ SUSPICIOUS REGION DETECTED
   }
   ```
4. Return warning với Base Address và Size

### Tại sao RWX memory nguy hiểm?
- **R** (Read): Đọc code
- **W** (Write): Ghi shellcode/payload
- **X** (Execute): Thực thi code động

→ Malware thường dùng để:
- Inject code vào process
- Unpack payload
- Code obfuscation
- Bypass static analysis

## 📁 Project Structure

```
WindowsBehaviorMonitor/
│
├── WindowsBehaviorMonitor.cpp      # Main application với menu
├── WindowsBehaviorMonitor.sln      # Visual Studio Solution
├── WindowsBehaviorMonitor.vcxproj  # Main project
│
├── ProcessManager.cpp              # Core scanning logic
├── ProcessManager.h                # ProcessManager interface
│
├── MockMalwareSim/
│   ├── MockMalwareSim.cpp         # Test target simulator
│   └── MockMalwareSim.vcxproj     # Simulator project
│
├── x64/Debug/
│   ├── WindowsBehaviorMonitor.exe # Main scanner
│   └── MockMalwareSim.exe         # Test tool
│
├── TESTING_GUIDE.md               # Detailed testing guide
├── Demo.ps1                       # Automated demo script
└── README.md                      # This file
```

## 🔧 Requirements

- **OS**: Windows 10/11 (x64)
- **Compiler**: Visual Studio 2022 (v143 toolset)
- **Platform**: x64 (để quét 64-bit processes)
- **Permissions**: Administrator (để scan protected processes)

## 🛠️ Build Instructions

```powershell
# Build all projects
msbuild WindowsBehaviorMonitor.sln /p:Configuration=Debug /p:Platform=x64 /t:Rebuild

# Or use Visual Studio:
# File → Open → Project/Solution → WindowsBehaviorMonitor.sln
# Build → Build Solution (Ctrl+Shift+B)
```

## 📚 Tài liệu tham khảo

- **TESTING_GUIDE.md**: Hướng dẫn test chi tiết
- **Demo.ps1**: Script tự động demo
- Code comments trong source files

## ⚠️ Lưu ý

### Bảo mật
- MockMalwareSim **không phải malware thật**, chỉ dùng cho testing
- Scanner có allowlist cho ứng dụng hợp lệ (browsers, JIT)
- Yêu cầu Administrator để scan protected processes

### Performance
- Full system scan có thể mất thời gian với nhiều processes
- Các process trong allowlist được skip tự động

### False Positives
- Browsers (Chrome, Edge) sử dụng RWX cho JIT → allowlisted
- Runtime environments (Java, Python) → allowlisted
- Game engines có thể trigger detection

## 🎓 Educational Purpose

Project này được tạo để:
- ✅ Hiểu cách EDR phát hiện malicious behavior
- ✅ Học Windows API (VirtualQueryEx, OpenProcess, etc.)
- ✅ Practice security development
- ✅ Test và verify detection logic

## 📝 License

Educational/Research purpose only.

---

**Developed by**: [Your Name]  
**Date**: December 13, 2025  
**Version**: 1.0
