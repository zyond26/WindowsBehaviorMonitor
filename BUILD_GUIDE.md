# 🔧 Hướng dẫn Build Project

## Bước 1: Cập nhật Visual Studio Project

Sau khi đã reorganize code, bạn cần add các file mới vào Visual Studio project:

### Cách 1: Sử dụng Visual Studio GUI

1. Mở **Visual Studio 2022**
2. Open Project: `WindowsBehaviorMonitor.sln`
3. Trong Solution Explorer, click chuột phải vào project → **Add** → **Existing Item**

#### Thêm PMM Module:
- `PMM/ProcessManager.h` (Add to Header Files)
- `PMM/ProcessManager.cpp` (Add to Source Files)

#### Thêm PFM Module:
- `PFM/RegistryMonitor.h` (Add to Header Files)
- `PFM/RegistryMonitor.cpp` (Add to Source Files)
- `PFM/StartupMonitor.h` (Add to Header Files)
- `PFM/StartupMonitor.cpp` (Add to Source Files)

#### Thêm NMM Module:
- `NMM/NetworkMonitor.h` (Add to Header Files)
- `NMM/NetworkMonitor.cpp` (Add to Source Files)

#### Thêm Common:
- `Common/Logger.h` (Add to Header Files)
- `Common/Logger.cpp` (Add to Source Files)
- `Common/EventStruct.h` (Add to Header Files)

4. Đảm bảo `WindowsBehaviorMonitor.cpp` là file main (không phải `WindowsBehaviorMonitor_OLD.cpp`)

### Cách 2: Edit .vcxproj file trực tiếp

Mở file `WindowsBehaviorMonitor.vcxproj` bằng text editor và thêm:

```xml
<ItemGroup>
  <ClInclude Include="PMM\ProcessManager.h" />
  <ClInclude Include="PFM\RegistryMonitor.h" />
  <ClInclude Include="PFM\StartupMonitor.h" />
  <ClInclude Include="NMM\NetworkMonitor.h" />
  <ClInclude Include="Common\Logger.h" />
  <ClInclude Include="Common\EventStruct.h" />
</ItemGroup>

<ItemGroup>
  <ClCompile Include="WindowsBehaviorMonitor.cpp" />
  <ClCompile Include="PMM\ProcessManager.cpp" />
  <ClCompile Include="PFM\RegistryMonitor.cpp" />
  <ClCompile Include="PFM\StartupMonitor.cpp" />
  <ClCompile Include="NMM\NetworkMonitor.cpp" />
  <ClCompile Include="Common\Logger.cpp" />
</ItemGroup>
```

## Bước 2: Configure Project Settings

### Include Directories
Đảm bảo project có thể tìm thấy headers:

1. Right-click project → **Properties**
2. **C/C++** → **General** → **Additional Include Directories**
3. Add: `$(ProjectDir);$(ProjectDir)PMM;$(ProjectDir)PFM;$(ProjectDir)NMM;$(ProjectDir)Common`

### Linker Settings
Đảm bảo link đúng thư viện:

1. **Linker** → **Input** → **Additional Dependencies**
2. Đảm bảo có: `iphlpapi.lib;ws2_32.lib;advapi32.lib;kernel32.lib`

### C++ Language Standard
1. **C/C++** → **Language** → **C++ Language Standard**
2. Chọn: **ISO C++17 Standard** hoặc mới hơn

## Bước 3: Build

### Debug Build
```powershell
# Trong Developer Command Prompt for VS 2022
msbuild WindowsBehaviorMonitor.sln /p:Configuration=Debug /p:Platform=x64
```

### Release Build
```powershell
msbuild WindowsBehaviorMonitor.sln /p:Configuration=Release /p:Platform=x64
```

### Hoặc trong Visual Studio:
- **Build** → **Build Solution** (Ctrl+Shift+B)

## Bước 4: Run

### Từ Visual Studio:
- **Debug** → **Start Debugging** (F5)
- Hoặc **Start Without Debugging** (Ctrl+F5)

### Từ Command Line:
```powershell
# Debug
.\x64\Debug\WindowsBehaviorMonitor.exe

# Release
.\x64\Release\WindowsBehaviorMonitor.exe
```

## ⚠️ Troubleshooting

### Lỗi: "Cannot open include file"
**Solution:** Kiểm tra Additional Include Directories đã đúng chưa

### Lỗi: "unresolved external symbol"
**Solution:** 
- Kiểm tra các `.cpp` files đã được add vào project chưa
- Kiểm tra linker dependencies (iphlpapi.lib, ws2_32.lib)

### Lỗi: "cannot convert from 'const char *' to 'LPCWSTR'"
**Solution:** Đảm bảo Character Set = **Unicode**

### Lỗi: Không tìm thấy file header
**Solution:**
- Kiểm tra đường dẫn trong `#include` statements
- Đảm bảo dùng relative paths từ project root

## 📝 Quick Checklist

- [ ] All `.h` files added to project
- [ ] All `.cpp` files added to project  
- [ ] Include directories configured
- [ ] Linker dependencies added
- [ ] C++17 or later selected
- [ ] Unicode character set
- [ ] Build thành công
- [ ] Run và test các module

## 🎯 Test sau khi Build

1. **PMM Module:**
   - List processes → Should show all running processes
   - Scan memory → Test với MockMalwareSim

2. **PFM Module:**
   - Start monitoring → Should monitor Registry & Startup
   - Thêm test registry key → Should detect

3. **NMM Module:**
   - Start monitoring → Should detect new TCP connections
   - Mở browser → Should see new connections

## 💡 Tips

- Build **Debug** khi đang develop để dễ debug
- Build **Release** khi hoàn thành để performance tốt hơn
- Run as **Administrator** để enable SeDebugPrivilege (PMM module)
- Check log file `WinBehaviorMonitor.log` nếu có vấn đề

---
**Good luck! Happy coding! 🚀**
