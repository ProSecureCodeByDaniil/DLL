# Лабораторная работа №3: DLL инъекции в С++ код и противодействие инъекции.

---

## 📋 Постановка задачи
1. Ознакомиться с механизмом инъекции на примере пользовательского проекта.
Пользовательский проект предоставлен в папке DLLInjectionLinkedIn
2. Реализовать защиту от инъекции.

---

## 📌 Отчет
### Введение
Ознакомление с механизмом DLL-инъекций в Windows с использованием Win32 API и реализация защиты от инъекций с помощью механизма `UpdateProcThreadAttribute` (политика блокировки немайкрософтовских библиотек).

### 1. Теоретическая часть: Механизм DLL-инъекции
### 1.1. Что такое DLL-инъекция?
DLL-инъекция — это метод внедрения кода в адресное пространство другого процесса. Поскольку DLL, загруженная в процесс, получает доступ ко всем его данным и памяти, этот метод может использоваться как для легитимных целей (расширение функциональности программ), так и для вредоносных (перехват данных, модификация поведения).

### 1.2. Принцип работы инъекции через `CreateRemoteThread`
Стандартный метод инъекции включает следующие шаги:
| Шаг | Действие | Функция Win32 API |
|-----|----------|-------------------|
| 0 | Открытие целевого процесса с необходимыми правами | `OpenProcess` | 
| 1 | Выделение памяти в адресном пространстве целевого процесса | `VirtualAllocEx` |
| 2 | Запись пути к DLL в выделенную память | `WriteProcessMemory` |
| 3 | Получение адреса функции `LoadLibraryA` в `kernel32.dll` | `GetProcAddress` |
| 4 | Создание удаленного потока, вызывающего `LoadLibraryA` | `CreateRemoteThread` |

### 1.3. Компоненты инъекционной атаки
В рамках лабораторной работы были созданы следующие компоненты:
| Компонент | Назначение |
|-----------|------------|
| **VirusDLL.dll** | Целевая вредоносная DLL, выполняющая атаку (отображение MessageBox с именем процесса) |
| **DllInjectorAsDll.dll** | DLL-версия инжектора, экспортирующая функцию `HelperFunc` (для вызова через `rundll32.exe`) |
| **DLLInjectorAsProcess.exe** | Исполняемый файл-инжектор, внедряющий DLL в целевой процесс |
| **DLLLoader.exe** | Простая программа для проверки загрузки VirusDLL.dll (LoadLibrary) |
| **TargetProcess.exe** | Процесс-жертва (простой счетчик), в который производится инъекция |

### 2. Практическая часть: Реализация DLL-инъекции
### 2.1. Вредоносная DLL (VirusDLL.dll)
Смотреть код находящийся в папке `DLLInjectionLinkedIn\VirusDLL\Source.cpp`.
### Компиляция:
cl /EHsc /Fe:VirusDLL.dll /LD Source.cpp /link psapi.lib

### 2.2. Процесс-жертва (TargetProcess.exe)
Смотреть код находящийся в папке `DLLInjectionLinkedIn\TargetProcess\Source.cpp`.
### Компиляция:
cl /EHsc /Fe:TargetProcess.exe Source.cpp

### 2.3. Инжектор (DLLInjectorAsProcess.exe)
Смотреть код находящийся в папке `DLLInjectionLinkedIn\DLLInjectorAsProcess\Source.cpp`.
### Компиляция:
cl /EHsc /Fe:DLLInjectorAsProcess.exe Source.cpp

### 2.4. Проверочный загрузчик DLL (DLLLoader.exe)
Смотреть код находящийся в папке `DLLInjectionLinkedIn\DLLLoader\Source.cpp`.
### Компиляция:
cl /EHsc /Fe:DLLLoader.exe Source.cpp

### 2.5. DLL-версия инжектора (DllInjectorAsDll.dll)
Смотреть код находящийся в папке `DLLInjectionLinkedIn\DllInjectorAsDll\Source.cpp`.
### Компиляция:
cl /EHsc /Fe:DllInjectorAsDll.dll /LD Source.cpp
### Запуск через rundll32.exe:
rundll32.exe DllInjectorAsDll.dll HelperFunc <PID_целевого_процесса>

### 2.6. Результат инъекции
При успешной инъекции с помощью **DLLInjectorAsProcess.exe** (или через **rundll32.exe** с **DllInjectorAsDll.dll**) в целевом процессе **TargetProcess.exe** появляется MessageBox с текстом "BOOM!".

### 3. Защита от DLL-инъекции
### 3.1. Метод защиты
Для защиты используется механизм `PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON`, который запрещает загрузку в процесс любых DLL, не подписанных Microsoft. Этот метод применяется через функцию `UpdateProcThreadAttribute` при создании процесса.
### Ограничения метода:
- Применяется только к новым процессам (при их создании через CreateProcess)
- DLL, подписанные Microsoft, по-прежнему могут загружаться
- Требуется Windows 8.1 / Windows Server 2012 или новее

### 3.2. Реализация защищенного процесса и самозащита процесса (TargetProcess.exe с защитой)
Смотреть код находящийся в папке `DLLInjectionLinkedIn\Защита от DLL\Source.cpp`.
### Примечание:
Для защиты самого себя процесс использует механизм перезапуска.

### Компиляция защищенного процесса:
cl /EHsc /Fe:TargetProcess_protected.exe Source.cpp /link psapi.lib

### 3.4. Проверка защиты
При попытке внедрения DLL в защищенный процесс **TargetProcess.exe** находящийся в папке DLLInjectionLinkedIn\Защита от DLL`:
### Запуск через терминал:
Попытка инъекции через **DLLInjectorAsProcess.exe**:
> DLLInjectorAsProcess.exe <PID_защищенного_процесса>

### Результат:
- CreateRemoteThread выполняется успешно
- Но LoadLibraryA не может загрузить неподписанную VirusDLL.dll
- MessageBox "BOOM!" не появляется

Попытка инъекции через `rundll32.exe` с **DllInjectorAsDll.dll** (Win + R):
> rundll32.exe DllInjectorAsDll.dll HelperFunc <PID_защищенного_процесса>
### Результат: 
Аналогичен предыдущей `DLLInjectorAsProcess.exe`.

### 4. Сводная таблица компонентов
| Имя файла | Назначение |
|-----------|------------|
| **VirusDLL.dll** | Вредоносная DLL, показывающая MessageBox |
| **DllInjectorAsDll.dll** | Инжектор в виде DLL (экспортирует HelperFunc) | 
| **DLLInjectorAsProcess.exe** | Инжектор в виде исполняемого файла |
| **DLLLoader.exe** | Тестовая программа для проверки VirusDLL.dll |
| **TargetProcess.exe** | Процесс-жертва (без защиты) |
| **Защита от DLL\TargetProcess.exe** | Процесс-жертва с защитой от инъекций |

### 5. Результаты эксперимента
| Эксперимент | Результат |
|-------------|-----------|
| Запуск **DLLLoader.exe** | Появляется MessageBox "BOOM!" от VirusDLL.dll |
| Запуск **TargetProcess.exe** без защиты | Счетчик работает нормально |
| Инъекция **VirusDLL.dll** через **DLLInjectorAsProcess.exe** в **TargetProcess.exe** | Появляется MessageBox "BOOM!" |
| Инъекция **VirusDLL.dll** через `rundll32.exe` + **DllInjectorAsDll.dll** в **TargetProcess.exe** | Появляется MessageBox "BOOM!" |
| Запуск **Защита от DLL\TargetProcess.exe** | Процесс запускается в защищенном режиме |
| Инъекция в **Защита от DLL\TargetProcess.exe** (любым способом) | Неудача — VirusDLL.dll не загружается |
| Загрузка системной DLL в **Защита от DLL\TargetProcess.exe** | Успешно (подписана Microsoft) |

### 6. Выводы
В ходе лабораторной работы были получены следующие результаты:
1. **Изучен механизм DLL-инъекции** через Windows API, включая работу с `OpenProcess`, `VirtualAllocEx`, `WriteProcessMemory`, `CreateRemoteThread`.
2. **Созданы рабочие прототипы**:
  - VirusDLL.dll — вредоносная DLL
  - DLLInjectorAsProcess.exe — исполняемый инжектор
  - DllInjectorAsDll.dll — DLL-версия инжектора для вызова через rundll32.exe
  - DLLLoader.exe — проверочный загрузчик
  - TargetProcess.exe — процесс-жертва
3. **Реализована защита от инъекций с использованием политики** `PROCESS_CREATION_MITIGATION_POLICY_BLOCK_NON_MICROSOFT_BINARIES_ALWAYS_ON` в `Защита от DLL\TargetProcess.exe`.
4. **Экспериментально подтверждена эффективность защиты** — попытки инъекции неподписанной `VirusDLL.dll` в защищенный процесс не увенчались успехом ни через `DLLInjectorAsProcess.exe`, ни через `rundll32.exe` + `DllInjectorAsDll.dll`.

### Ограничения метода защиты
- Защищает только процессы, созданные после применения политики
- Не предотвращает загрузку Microsoft-подписанных DLL
- Не защищает от инъекций через другие методы (например, SetWindowsHookEx)
- Требуется Windows 8.1 / Windows Server 2012 или новее (со старым Internet Explorer)

### 7. Список использованных API-функций
| Функция | Назначение |
|---------|------------|
| `OpenProcess` | Открытие дескриптора процесса |
| `VirtualAllocEx` | Выделение памяти в чужом процессе |
| `WriteProcessMemory` | Запись данных в память чужого процесса |
| `CreateRemoteThread` | Создание потока в чужом процессе |
| `LoadLibraryA` / `LoadLibraryW` | Загрузка DLL (используется как точка входа) |
| `GetProcAddress` | Получение адреса функции из DLL |
| `GetModuleHandle` | Получение дескриптора загруженного модуля |
| `InitializeProcThreadAttributeList` | Инициализация списка атрибутов процесса |
| `UpdateProcThreadAttribute` | Обновление атрибутов процесса |
| `CreateProcessA` | Создание нового процесса |
| `GetModuleFileNameA` | Получение пути к исполняемому файлу |
| `GetCurrentProcessId` | Получение PID текущего процесса |
