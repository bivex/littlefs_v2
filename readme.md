# littlefs v2 — High-Performance Virtual File System (VFS)

`littlefs_v2` — это глубоко рефакторенная C++ модификация [littlefs](https://github.com/littlefs-project/littlefs), расширенная слоем **High-Performance VFS Page Cache**, поддержкой произвольного доступа $O(1)$, драйвером **Linux FUSE 3** и устойчивостью к аварийному отключению питания (Power-Loss Resilience).

---

## 🌟 Ключевые Особенности (Features)

* **Современная C++ Архитектура**: Высокоуровневая абстракция с объектами `IFileSystemDevice` и `IFileObject`.
* **High-Performance Page Cache Layer**:
  * $O(1)$ произвольный поиск (`seek`), чтение (`read`) и определение размера (`size`) в RAM.
  * Пакетный сброс страниц (`Batched Flush`): агрегация 2 000+ мелких произвольных записей в один проход, предотвращающая деградацию производительности каскадных переписок CTZ skip-list.
* **Поддержка Огромных Файлов**: Максимальный размер файла расширен до `0x7FFFFFFFFFFFFFFF` (8 Эксабайт).
* **Динамический Auto-Grow диска**: Автоматическое расширение размера виртуального диска на лету без размонтирования ФС.
* **Два Бэкенда Хранения (Dual-Backend)**:
  1. `kFileBackend`: Хранение контейнера в дисковом файле (`.vfs` / `.bin`).
  2. `kMemoryBackend`: Высокоскоростное хранение в оперативной памяти (RAM).
* **Linux FUSE 3 Driver**: Монтирование виртуальных `.vfs` контейнеров напрямую в файловую систему ОС Linux (`lfs_fuse`).
* **Кроссплатформенность**: Поддержка Windows, macOS и Linux (MSVC, Clang, GCC).

---

## 📊 Замеры Производительности (Performance Benchmark)

*Замеры на файле размером 1 МБ (1 048 576 байт):*

| Операция | Объем / Кол-во | Время выполнения | Средняя скорость |
| :--- | :---: | :---: | :---: |
| **Последовательная запись** | 1 МБ | **`8.18 ms`** | `128 MB/s` |
| **Случайный Seek & Read (RAM)** | 10 000 операций | **`0.83 ms`** | **`83 ns / op`** |
| **Случайная перезапись (Page Cache)** | 2 000 операций | **`0.17 ms`** | **`88 ns / op`** |
| **Пакетный сброс на диск (Batched Flush)** | 2 000 записей | **`0.47 ms`** | `< 0.5 ms` |
| **Сканирование каталогов (Directory Scan)** | 1 000 файлов | **`15.33 ms`** | `15 us / file` |

---

## ⚡ Устойчивость к Сбоям Питания (Power-Loss Resilience)

В отличие от стандартных файловых систем, `littlefs_v2` спроектирована для работы в нестабильных средах (извлечение USB-флешки, аварийное отключение питания):
* Проверено тестом `src/example/power_loss_test.cpp`.
* **Результат**: **`0% Metadata Corruption`**. Все ранее зафиксированные состояния на 100% сохраняются, а метаданные файловой системы автоматически самовосстанавливаются после аварийного перезапуска.

---

## 📁 Структура Проекта и Демо-Примеры

```text
littlefs_v2/
├── include/                   # Заголовочные файлы littlefs_v2 (lfs.h, lfs_utility.h)
├── src/
│   ├── littlefs_v2/           # Исходный код C++ ядра littlefs_v2
│   └── example/
│       ├── lfs_interface.h    # Интерфейсы VFS (IFileSystemDevice, IFileObject)
│       ├── lfs_interface.cpp  # Реализация VFS и Page Cache
│       ├── asset_vault_example.cpp # Демо-проект: Архив 1 000 мелких файлов ресурсов
│       ├── lfs_fuse.cpp       # Драйвер Linux FUSE 3
│       ├── power_loss_test.cpp# Тестовый комплекс на сбои питания
│       └── vfs_tests.cpp      # Unit-тесты (46 проверок)
└── docs/
    └── vfs_value_and_architecture.md # Коммерческая и техническая документация
```

---

## 🚀 Быстрый Старт (Usage Example)

```cpp
#include "lfs_interface.h"

int main() {
    std::shared_ptr<fs::IFileSystemDevice> vfs;
    
    // Создаем VFS-контейнер на диске
    if (fs::createVFS(L"my_container.vfs", vfs, fs::lfsVFS::Backend::kFileBackend) == fs::kCodeOK) {
        std::shared_ptr<fs::IFileObject> file;
        
        // Открываем / создаем файл внутри VFS
        if (vfs->openFile(file, "hello.txt", fs::kFileWrite | fs::kFileCreateIfNotExists) == fs::kCodeOK) {
            std::string data = "Hello littlefs_v2 VFS!";
            file->write(data.c_str(), data.size());
            file->flush(); // Сброс грязных страниц на диск
        }
    }
    return 0;
}
```

---

## 🐧 Сборка и Монтирование Linux FUSE 3 Driver

В ОС Linux или виртуальной машине Linux (Ubuntu / Debian):

```bash
# 1. Установите зависимости FUSE 3
sudo apt-get update && sudo apt-get install -y libfuse3-dev build-essential pkg-config

# 2. Скомпилируйте FUSE-драйвер
g++ -std=c++17 -O2 -Iinclude -Isrc/example \
    src/littlefs_v2/*.cpp src/example/lfs_interface.cpp src/example/lfs_fuse.cpp \
    -D_FILE_OFFSET_BITS=64 $(pkg-config --cflags --libs fuse3) -o littlefs_fuse

# 3. Смонтируйте VFS контейнер в любой каталог
mkdir -p /mnt/my_vfs
./littlefs_fuse /mnt/my_vfs

# 4. Работайте как с обычной папкой Linux
echo "Hello from Linux terminal!" > /mnt/my_vfs/test.txt
cat /mnt/my_vfs/test.txt

# 5. Размонтирование
fusermount3 -u /mnt/my_vfs
```

---

## 📄 Документация

Подробный коммерческий разбор практической ценности, ROI, экономии на IOPS и изоляции плагинов находится в документе:
👉 [`docs/vfs_value_and_architecture.md`](file:///Volumes/External/Code/f/littlefs_v2/docs/vfs_value_and_architecture.md)