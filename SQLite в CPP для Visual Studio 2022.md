### **🚀 Краткая инструкция по установке SQLite в C++ (Visual Studio 2022) с vcpkg**  

#### **1. Установили vcpkg**  
```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
```
*(`.\` в PowerShell обязателен, иначе команда не найдётся.)*

---

#### **2. Интегрировали vcpkg с Visual Studio**  
```powershell
.\vcpkg integrate install
```
Теперь Visual Studio автоматически подключает установленные библиотеки.

---

#### **3. Установили SQLite**  
```powershell
.\vcpkg install sqlite3:x64-windows
```
*(Если нужна 32-битная версия, замените на `sqlite3:x86-windows`.)*

---

#### **4. Проверили подключение SQLite в C++ коде**
Создали C++ проект в **Visual Studio 2022** и добавили код:  
```cpp
#include <iostream>
#include <sqlite3.h>

int main() {
    sqlite3* db;
    int rc = sqlite3_open("test.db", &db);
    
    if (rc) {
        std::cerr << "Ошибка при открытии базы данных: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } else {
        std::cout << "База данных успешно открыта!" << std::endl;
    }

    sqlite3_close(db);
    return 0;
}
```
**Собрали (`Ctrl + Shift + B`) и запустили (`F5`) в Visual Studio**.  

---

#### **5. (Опционально) Добавили vcpkg в системный PATH**  
Чтобы не писать `.\vcpkg`, добавили `C:\vcpkg` в **Переменные среды → Path**.  

---

✅ **Готово! Теперь SQLite работает в C++ проектах Visual Studio 2022.** 🚀