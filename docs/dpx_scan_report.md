# 🔍 Software Design Pattern Detection Report

> **Project:** `/Volumes/External/Code/littlefs_v2`  
> **Scanned Files:** 25  
> **Total Detections:** 54  
> **Duration:** 0.820s  

---

## 📊 Summary by Category

| Category | Detections Count |
| :--- | :---: |
| **CREATIONAL** | 1 |
| **STRUCTURAL** | 1 |
| **BEHAVIORAL** | 4 |
| **PRINCIPLE** | 48 |

---

## 📋 Identified Design Patterns

### #1 KISS on kiss_cyclomatic_complexity `lfs_dir_getslice`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_getslice' has 8 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_getslice' has high cyclomatic complexity (8 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_

---

### #2 KISS on kiss_cyclomatic_complexity `lfs_dir_find`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_find' has 15 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_find' has high cyclomatic complexity (15 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_

---

### #3 KISS on kiss_cyclomatic_complexity `lfs_init`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_init' has 15 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_init' has high cyclomatic complexity (15 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_

---

### #4 KISS on kiss_cyclomatic_complexity `lfs_raw_remove`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_raw_remove' has 12 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_raw_remove' has high cyclomatic complexity (12 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_

---

### #5 KISS on kiss_cyclomatic_complexity `lfs_raw_rename`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_raw_rename' has 21 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_raw_rename' has high cyclomatic complexity (21 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_

---

### #6 KISS on kiss_cyclomatic_complexity `lfs_raw_mount`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_raw_mount' has 29 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_raw_mount' has high cyclomatic complexity (29 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_general.cpp:1:1`)_

---

### #7 KISS on kiss_cyclomatic_complexity `lfs_dir_commit_crc`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_commit_crc' has 12 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_commit_crc' has high cyclomatic complexity (12 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #8 KISS on kiss_cyclomatic_complexity `lfs_dir_compact`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_compact' has 22 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_compact' has high cyclomatic complexity (22 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #9 KISS on kiss_cyclomatic_complexity `lfs_dir_splittingcompact`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_splittingcompact' has 12 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_splittingcompact' has high cyclomatic complexity (12 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #10 KISS on kiss_cyclomatic_complexity `lfs_dir_relocating_commit`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_relocating_commit' has 28 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_relocating_commit' has high cyclomatic complexity (28 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #11 KISS on kiss_cyclomatic_complexity `lfs_dir_orphaning_commit`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_orphaning_commit' has 29 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_orphaning_commit' has high cyclomatic complexity (29 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #12 KISS on kiss_cyclomatic_complexity `lfs_bd_read`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_bd_read' has 9 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_bd_read' has high cyclomatic complexity (9 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_

---

### #13 KISS on kiss_cyclomatic_complexity `lfs_ctz_extend`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_ctz_extend' has 15 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_ctz_extend' has high cyclomatic complexity (15 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`)_

---

### #14 KISS on kiss_cyclomatic_complexity `lfs_file_rawopencfg`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_rawopencfg' has 22 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_rawopencfg' has high cyclomatic complexity (22 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #15 KISS on kiss_cyclomatic_complexity `lfs_file_relocate`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_relocate' has 10 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_relocate' has high cyclomatic complexity (10 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #16 KISS on kiss_cyclomatic_complexity `lfs_file_flush`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_flush' has 12 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_flush' has high cyclomatic complexity (12 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #17 KISS on kiss_cyclomatic_complexity `lfs_file_flushedread`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_flushedread' has 8 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_flushedread' has high cyclomatic complexity (8 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #18 KISS on kiss_cyclomatic_complexity `lfs_file_flushedwrite`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_flushedwrite' has 12 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_flushedwrite' has high cyclomatic complexity (12 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #19 KISS on kiss_cyclomatic_complexity `lfs_file_rawwrite`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_rawwrite' has 8 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_rawwrite' has high cyclomatic complexity (8 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #20 KISS on kiss_cyclomatic_complexity `lfs_file_rawseek`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_rawseek' has 10 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_rawseek' has high cyclomatic complexity (10 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #21 KISS on kiss_cyclomatic_complexity `lfs_file_rawtruncate`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_file_rawtruncate' has 12 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_file_rawtruncate' has high cyclomatic complexity (12 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file.cpp:1:1`)_

---

### #22 KISS on kiss_cyclomatic_complexity `lfs_dir_rawcreate`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_rawcreate' has 12 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_rawcreate' has high cyclomatic complexity (12 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp:1:1`)_

---

### #23 KISS on kiss_cyclomatic_complexity `lfs_dir_rawread`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_dir_rawread' has 8 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_dir_rawread' has high cyclomatic complexity (8 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_directory.cpp:1:1`)_

---

### #24 KISS on kiss_cyclomatic_complexity `lfs_fs_deorphan`
- **Confidence:** 88% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_operations.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_operations.cpp)
- **Summary:** KISS Violation (High Complexity): Method 'lfs_fs_deorphan' has 19 control flow branches

#### 🔎 Evidence Trail:
- **+70%** `[KISS_KISS_HIGH_CYCLOMATIC_COMPLEXITY]` Method 'lfs_fs_deorphan' has high cyclomatic complexity (19 branch points), violating KISS _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_operations.cpp:1:1`)_
- **+35%** `[KISS_KISS_DECOMPOSITION_NEEDED]` Complex nested conditionals are difficult to test and maintain; decompose into smaller functions _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_operations.cpp:1:1`)_

---

### #25 OPEN_CLOSED on ocp_polymorphic_hierarchy `IBlockDevice`
- **Confidence:** 87% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- **Summary:** OCP Adherence: Interface 'IBlockDevice' supports open extension with 3 implementations

#### 🔎 Evidence Trail:
- **+70%** `[OPEN_CLOSED_OCP_POLYMORPHIC_ABSTRACTION]` Abstract interface 'IBlockDevice' enables open extension through 3 polymorphic implementations: MemoryBlockDevice, FileBlockDevice, BlockDeviceDecorator _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_
- **+35%** `[OPEN_CLOSED_OCP_EXTENSIBLE_DESIGN]` New behaviors can be added by implementing the interface without modifying existing consumers _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_

**Related Locations:**
- [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)

---

### #26 SINGLE_RESPONSIBILITY on god_class_srp_violation `lfsVFS`
- **Confidence:** 86% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h)
- **Summary:** SRP Violation (God Class): 'lfsVFS' mixes 2 concerns across 13 methods

#### 🔎 Evidence Trail:
- **+50%** `[SINGLE_RESPONSIBILITY_SRP_MIXED_CONCERNS]` Class 'lfsVFS' mixes 2 disparate concerns (persistence (2 methods), http_web (1 methods)), violating SRP _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_
- **+40%** `[SINGLE_RESPONSIBILITY_SRP_HIGH_METHOD_COUNT]` High method count (13 methods) indicates bloated class responsibility _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_
- **+25%** `[SINGLE_RESPONSIBILITY_SRP_HIGH_FIELD_COUNT]` High field count (6 fields) suggests multi-purpose state holder _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_

---

### #27 INTERFACE_SEGREGATION on fat_interface_isp_violation `FileBlockDevice`
- **Confidence:** 86% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- **Summary:** ISP Violation (Fat Interface): 'FileBlockDevice' has 14 methods; should be split into smaller role interfaces

#### 🔎 Evidence Trail:
- **+65%** `[INTERFACE_SEGREGATION_ISP_FAT_INTERFACE]` Interface 'FileBlockDevice' is a Fat Interface defining 14 methods (FileBlockDevice, ~FileBlockDevice, is_open, read, write, erase...), violating ISP _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_
- **+35%** `[INTERFACE_SEGREGATION_ISP_UNNEEDED_DEPENDENCY]` Clients and implementors are forced to depend on methods they may not use _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_

---

### #28 INTERFACE_SEGREGATION on fat_interface_isp_violation `VFSFileObject`
- **Confidence:** 86% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp)
- **Summary:** ISP Violation (Fat Interface): 'VFSFileObject' has 13 methods; should be split into smaller role interfaces

#### 🔎 Evidence Trail:
- **+65%** `[INTERFACE_SEGREGATION_ISP_FAT_INTERFACE]` Interface 'VFSFileObject' is a Fat Interface defining 13 methods (VFSFileObject, init_file_state, ~VFSFileObject, close, load_page_if_needed, flush_internal...), violating ISP _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`)_
- **+35%** `[INTERFACE_SEGREGATION_ISP_UNNEEDED_DEPENDENCY]` Clients and implementors are forced to depend on methods they may not use _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`)_

---

### #29 INTERFACE_SEGREGATION on fat_interface_isp_violation `IFileObject`
- **Confidence:** 86% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h)
- **Summary:** ISP Violation (Fat Interface): 'IFileObject' has 8 methods; should be split into smaller role interfaces

#### 🔎 Evidence Trail:
- **+65%** `[INTERFACE_SEGREGATION_ISP_FAT_INTERFACE]` Interface 'IFileObject' is a Fat Interface defining 8 methods (read, write, truncate, seek, tell, size...), violating ISP _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_
- **+35%** `[INTERFACE_SEGREGATION_ISP_UNNEEDED_DEPENDENCY]` Clients and implementors are forced to depend on methods they may not use _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_

---

### #30 INTERFACE_SEGREGATION on fat_interface_isp_violation `IFileSystemDevice`
- **Confidence:** 86% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h)
- **Summary:** ISP Violation (Fat Interface): 'IFileSystemDevice' has 10 methods; should be split into smaller role interfaces

#### 🔎 Evidence Trail:
- **+65%** `[INTERFACE_SEGREGATION_ISP_FAT_INTERFACE]` Interface 'IFileSystemDevice' is a Fat Interface defining 10 methods (dir, openFile, existsFile, deleteFile, deleteDirectory, listDir...), violating ISP _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_
- **+35%** `[INTERFACE_SEGREGATION_ISP_UNNEEDED_DEPENDENCY]` Clients and implementors are forced to depend on methods they may not use _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_

---

### #31 KISS on kiss_complexity_parameters `lfs_fuse_readdir`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_fuse.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_fuse.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_fuse_readdir' takes 6 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_fuse_readdir' has 6 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_fuse.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_fuse.cpp:1:1`)_

---

### #32 KISS on kiss_complexity_parameters `lfs_dir_getslice`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_dir_getslice' takes 7 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_dir_getslice' has 7 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_

---

### #33 KISS on kiss_complexity_parameters `lfs_dir_getread`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_dir_getread' takes 10 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_dir_getread' has 10 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_metadata.cpp:1:1`)_

---

### #34 KISS on kiss_complexity_parameters `lfs_dir_split`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_dir_split' takes 7 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_dir_split' has 7 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #35 KISS on kiss_complexity_parameters `lfs_dir_compact`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_dir_compact' takes 7 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_dir_compact' has 7 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #36 KISS on kiss_complexity_parameters `lfs_dir_splittingcompact`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_dir_splittingcompact' takes 7 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_dir_splittingcompact' has 7 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #37 KISS on kiss_complexity_parameters `lfs_dir_relocating_commit`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_dir_relocating_commit' takes 6 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_dir_relocating_commit' has 6 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_commit.cpp:1:1`)_

---

### #38 KISS on kiss_complexity_parameters `lfs_bd_read`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_bd_read' takes 8 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_bd_read' has 8 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_

---

### #39 KISS on kiss_complexity_parameters `lfs_bd_cmp`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_bd_cmp' takes 8 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_bd_cmp' has 8 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_

---

### #40 KISS on kiss_complexity_parameters `lfs_bd_write`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_bd_write' takes 8 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_bd_write' has 8 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_device.cpp:1:1`)_

---

### #41 KISS on kiss_complexity_parameters `lfs_ctz_find`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_ctz_find' takes 8 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_ctz_find' has 8 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`)_

---

### #42 KISS on kiss_complexity_parameters `lfs_ctz_extend`
- **Confidence:** 85% (🟢 `VERY_HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp)
- **Summary:** KISS Violation (Long Parameter List): Method 'lfs_ctz_extend' takes 7 parameters

#### 🔎 Evidence Trail:
- **+65%** `[KISS_KISS_LONG_PARAMETER_LIST]` Method 'lfs_ctz_extend' has 7 parameters, violating KISS (Long Parameter List) _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`)_
- **+35%** `[KISS_KISS_PARAMETER_OBJECT_RECOMMENDED]` Excessive parameters increase cognitive load and error probability; consider a Parameter Object / Builder _(at `/Volumes/External/Code/littlefs_v2/src/littlefs_v2/lfs_file_index.cpp:1:1`)_

---

### #43 DEPENDENCY_INVERSION on dip_interface_dependency `VFSFileObject`
- **Confidence:** 83% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp)
- **Summary:** DIP Adherence: 'VFSFileObject' depends on interface abstraction(s) (ErrorCode)

#### 🔎 Evidence Trail:
- **+60%** `[DEPENDENCY_INVERSION_DIP_INJECTED_ABSTRACTION]` Class 'VFSFileObject' depends on abstracted interface(s): ErrorCode adhering to DIP _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`)_
- **+35%** `[DEPENDENCY_INVERSION_DIP_DECOUPLED_ARCHITECTURE]` Core domain logic is decoupled from infrastructure details via Dependency Injection _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`)_

---

### #44 DEPENDENCY_INVERSION on dip_interface_dependency `lfs_disk_offset_t`
- **Confidence:** 83% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs.h)
- **Summary:** DIP Adherence: 'lfs_disk_offset_t' depends on interface abstraction(s) (CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice)

#### 🔎 Evidence Trail:
- **+60%** `[DEPENDENCY_INVERSION_DIP_INJECTED_ABSTRACTION]` Class 'lfs_disk_offset_t' depends on abstracted interface(s): CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice adhering to DIP _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_
- **+35%** `[DEPENDENCY_INVERSION_DIP_DECOUPLED_ARCHITECTURE]` Core domain logic is decoupled from infrastructure details via Dependency Injection _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_

---

### #45 DEPENDENCY_INVERSION on dip_interface_dependency `lfs_commit_t`
- **Confidence:** 83% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs.h)
- **Summary:** DIP Adherence: 'lfs_commit_t' depends on interface abstraction(s) (CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice)

#### 🔎 Evidence Trail:
- **+60%** `[DEPENDENCY_INVERSION_DIP_INJECTED_ABSTRACTION]` Class 'lfs_commit_t' depends on abstracted interface(s): CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice adhering to DIP _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_
- **+35%** `[DEPENDENCY_INVERSION_DIP_DECOUPLED_ARCHITECTURE]` Core domain logic is decoupled from infrastructure details via Dependency Injection _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_

---

### #46 DEPENDENCY_INVERSION on dip_interface_dependency `lfs_cache_t`
- **Confidence:** 83% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs.h)
- **Summary:** DIP Adherence: 'lfs_cache_t' depends on interface abstraction(s) (CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice)

#### 🔎 Evidence Trail:
- **+60%** `[DEPENDENCY_INVERSION_DIP_INJECTED_ABSTRACTION]` Class 'lfs_cache_t' depends on abstracted interface(s): CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice adhering to DIP _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_
- **+35%** `[DEPENDENCY_INVERSION_DIP_DECOUPLED_ARCHITECTURE]` Core domain logic is decoupled from infrastructure details via Dependency Injection _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_

---

### #47 DEPENDENCY_INVERSION on dip_interface_dependency `lfs_file_t`
- **Confidence:** 83% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs.h)
- **Summary:** DIP Adherence: 'lfs_file_t' depends on interface abstraction(s) (CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice)

#### 🔎 Evidence Trail:
- **+60%** `[DEPENDENCY_INVERSION_DIP_INJECTED_ABSTRACTION]` Class 'lfs_file_t' depends on abstracted interface(s): CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, StatisticsBlockDevice adhering to DIP _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_
- **+35%** `[DEPENDENCY_INVERSION_DIP_DECOUPLED_ARCHITECTURE]` Core domain logic is decoupled from infrastructure details via Dependency Injection _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_

---

### #48 DEPENDENCY_INVERSION on dip_interface_dependency `lfs_free_t`
- **Confidence:** 83% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs.h)
- **Summary:** DIP Adherence: 'lfs_free_t' depends on interface abstraction(s) (CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, IFileObject, IFileSystemDevice, IVFSEventListener, StatisticsBlockDevice, VFSBuilder, VFSFileObject)

#### 🔎 Evidence Trail:
- **+60%** `[DEPENDENCY_INVERSION_DIP_INJECTED_ABSTRACTION]` Class 'lfs_free_t' depends on abstracted interface(s): CryptoBlockDevice, FaultInjectBlockDevice, FileBlockDevice, IBlockDevice, IFileObject, IFileSystemDevice, IVFSEventListener, StatisticsBlockDevice, VFSBuilder, VFSFileObject adhering to DIP _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_
- **+35%** `[DEPENDENCY_INVERSION_DIP_DECOUPLED_ARCHITECTURE]` Core domain logic is decoupled from infrastructure details via Dependency Injection _(at `/Volumes/External/Code/littlefs_v2/include/lfs.h:1:1`)_

---

### #49 STRATEGY on protocol_strategy `IBlockDevice`
- **Confidence:** 81% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- **Summary:** Strategy pattern: protocol 'IBlockDevice' with 3 interchangeable concrete implementations

#### 🔎 Evidence Trail:
- **+45%** `[STRATEGY_PROTOCOL_STRATEGY_INTERFACE]` Protocol 'IBlockDevice' defines strategy interface with methods: read, write, erase, sync, get_block_size, get_block_count, set_block_count _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_
- **+25%** `[STRATEGY_RECORD_STRATEGY_IMPL]` Record 'MemoryBlockDevice' provides concrete strategy implementation for protocol 'IBlockDevice' _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_
- **+25%** `[STRATEGY_RECORD_STRATEGY_IMPL]` Record 'FileBlockDevice' provides concrete strategy implementation for protocol 'IBlockDevice' _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_
- **+25%** `[STRATEGY_RECORD_STRATEGY_IMPL]` Record 'BlockDeviceDecorator' provides concrete strategy implementation for protocol 'IBlockDevice' _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_

**Related Locations:**
- [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)

---

### #50 DECORATOR on cpp_decorator_class `BlockDeviceDecorator`
- **Confidence:** 81% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)
- **Summary:** Decorator pattern: class 'BlockDeviceDecorator' dynamically augments component behavior via wrapping

#### 🔎 Evidence Trail:
- **+50%** `[DECORATOR_DECORATOR_NAMING]` Class 'BlockDeviceDecorator' follows Decorator pattern naming convention _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_
- **+45%** `[DECORATOR_DECORATOR_IMPLEMENTS_COMPONENT]` Implements decorated component interface(s): IBlockDevice _(at `/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`)_

**Related Locations:**
- [`/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h:1:1`](/Volumes/External/Code/littlefs_v2/include/lfs_block_device.h)

---

### #51 TEMPLATE_METHOD on template_method_protocol `IFileObject`
- **Confidence:** 80% (🔵 `HIGH`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h)
- **Summary:** Template Method pattern: 'IFileObject' defines skeleton of algorithm in base class

#### 🔎 Evidence Trail:
- **+55%** `[TEMPLATE_METHOD_TEMPLATE_METHOD_SKELETON]` Class 'IFileObject' defines template algorithm skeleton with primitive operations: read, write, truncate, seek, tell, size, flush, close _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_
- **+35%** `[TEMPLATE_METHOD_CONCRETE_TEMPLATE_IMPL]` Subclass 'VFSFileObject' overrides primitive step operations without changing algorithm structure _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`)_

**Related Locations:**
- [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp)

---

### #52 OBSERVER on observer_protocol `IVFSEventListener`
- **Confidence:** 69% (🟡 `MEDIUM`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h)
- **Summary:** Observer pattern: observer interface 'IVFSEventListener' implemented by 0 observer records

#### 🔎 Evidence Trail:
- **+55%** `[OBSERVER_OBSERVER_INTERFACE]` Protocol 'IVFSEventListener' defines Observer interface with callback methods: onVFSEvent _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_

---

### #53 TEMPLATE_METHOD on template_method_protocol `VFSFileObject`
- **Confidence:** 69% (🟡 `MEDIUM`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp)
- **Summary:** Template Method pattern: 'VFSFileObject' defines skeleton of algorithm in base class

#### 🔎 Evidence Trail:
- **+55%** `[TEMPLATE_METHOD_TEMPLATE_METHOD_SKELETON]` Class 'VFSFileObject' defines template algorithm skeleton with primitive operations: VFSFileObject, init_file_state, ~VFSFileObject, close, load_page_if_needed, flush_internal, read, write, truncate, seek, tell, size, flush _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.cpp:1:1`)_

---

### #54 BUILDER on builder_protocol `VFSBuilder`
- **Confidence:** 69% (🟡 `MEDIUM`)
- **Primary Location:** [`/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`](/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h)
- **Summary:** Builder pattern: protocol 'VFSBuilder' defines construction steps implemented by 0 concrete builders

#### 🔎 Evidence Trail:
- **+55%** `[BUILDER_BUILDER_PROTOCOL]` Protocol 'VFSBuilder' defines builder construction interface with methods: withCrypto _(at `/Volumes/External/Code/littlefs_v2/src/example/lfs_interface.h:1:1`)_

---
