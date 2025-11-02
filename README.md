# rxsnr - Regular Expression Search and Replace

**rxsnr** is a powerful command-line utility for searching and replacing text using regular expressions. It supports multiple text encodings (ASCII, UTF-8, UTF-16) and provides flexible pattern matching capabilities.

## Features

- **Regular Expression Support**: Full support for POSIX-style regular expressions
- **Multiple Encodings**: Automatic detection and processing of ASCII, UTF-8, and UTF-16 files
- **Flexible Input**: Command-line patterns or pattern files
- **Stream Processing**: Line-by-line or streaming mode processing
- **Output Control**: Various output formats and encoding options
- **Pattern Chaining**: Apply multiple patterns sequentially

## Compilation

This project has been migrated from C++Builder to **Visual Studio 2022**.

### Requirements
- Visual Studio 2022 (v143 toolset)
- Windows SDK 10.0
- C++17 standard support

### Build Instructions
```bash
# Open Developer Command Prompt for VS 2022
cd rx_vs2022
msbuild rxsnr.sln /p:Configuration=Release /p:Platform=x64
```

Or open `rxsnr.sln` in Visual Studio 2022 and build the solution.

## Usage

### Command Line Syntax

```bash
rxsnr [-options] /search/replace[/num] <input> <output>
rxsnr [-options] /search/replace/num/input/[+]output
rxsnr [-options] rules.txt <input> <output>
rxsnr [-options] rules.txt input [+]output
```

### Options

- `-h` - Show help screen
- `-H` - Show full help with regular expression syntax
- `-d` - Debug mode (show processing information)
- `-b` - Break after first match in pattern list
- `-u` - Unicode (UTF-16) output
- `-8` - UTF-8 output
- `-s` - Stream mode (ignore input line endings)

### Examples

#### Basic Search and Replace
```bash
# Replace "hello" with "hi" in file.txt
rxsnr "/hello/hi/" input.txt output.txt

# Case-sensitive replacement with debug output
rxsnr -d "/Hello/Hi/" input.txt output.txt
```

#### Using Pattern Files
```bash
# Create a pattern file
echo "/old/new/" > patterns.txt
echo "/foo/bar/" >> patterns.txt

# Apply patterns from file
rxsnr patterns.txt input.txt output.txt
```

#### Advanced Patterns
```bash
# Replace digits with "NUM" + the digit
rxsnr "/([0-9])/NUM\\1/" input.txt output.txt

# Replace email addresses
rxsnr "/([a-zA-Z0-9._%-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,})/EMAIL/" input.txt output.txt
```

#### Output Options
```bash
# Append to existing file
rxsnr "/pattern/replacement/" input.txt +output.txt

# UTF-8 output
rxsnr -8 "/pattern/replacement/" input.txt output.txt

# UTF-16 output
rxsnr -u "/pattern/replacement/" input.txt output.txt
```

### Pattern File Format

Pattern files contain one or more search/replace rules:
```
/search_pattern/replacement_text/[options]
```

Lines starting with `;` are treated as comments.

## Regular Expression Syntax

### Search Patterns

| Pattern | Description |
|---------|-------------|
| `^` | Match start of line |
| `$` | Match end of line |
| `.` | Match any character |
| `*` | Zero or more occurrences |
| `+` | One or more occurrences |
| `?` | Optional (zero or one occurrence) |
| `[abc]` | Match any character in brackets |
| `[^abc]` | Match any character NOT in brackets |
| `[a-z]` | Character range |
| `(pattern)` | Group (up to 16 groups supported) |
| `pattern1|pattern2` | Alternation (OR) |
| `\\` | Escape special characters |

### Replacement Patterns

| Pattern | Description |
|---------|-------------|
| `\\0` to `\\9` | Recall matched groups 1-10 |
| `\\A` to `\\F` | Recall matched groups 11-16 |
| `&` | Entire matched pattern |
| `\\n`, `\\t`, `\\r` | Newline, tab, carriage return |
| `\\\\` | Literal backslash |

## File Encoding Support

rxsnr automatically detects input file encoding:
- **UTF-16**: Files starting with BOM (0xFF 0xFE)
- **UTF-8**: Files starting with BOM (0xEF 0xBB 0xBF)  
- **ASCII/ANSI**: Default encoding for other files

Output encoding can be specified using command-line options.

## Project Structure

```
rx_vs2022/
├── rxsearch.cpp      # Main application source
├── RegExpClass.h     # Regular expression engine header
├── RegExpClass.cpp   # Regular expression engine implementation
├── rxsnr.vcxproj     # Visual Studio project file
├── rxsnr.sln         # Visual Studio solution file
└── README.md         # This file
```

## Migration Notes

This project was successfully migrated from C++Builder to Visual Studio 2022. Key changes include:

- Replaced C++Builder-specific headers and pragmas
- Updated argument parsing to use standard `argc`/`argv`
- Added proper Visual Studio project configuration
- Maintained original algorithm and functionality

## License

Copyright (c) 2014 Dmitry Orlov (dimorlus@gmail.com)

---

# rxsnr - Поиск и Замена с Регулярными Выражениями

**rxsnr** — мощная консольная утилита для поиска и замены текста с использованием регулярных выражений. Поддерживает множественные кодировки текста (ASCII, UTF-8, UTF-16) и предоставляет гибкие возможности сопоставления шаблонов.

## Возможности

- **Поддержка регулярных выражений**: Полная поддержка регулярных выражений в стиле POSIX
- **Множественные кодировки**: Автоматическое определение и обработка файлов ASCII, UTF-8 и UTF-16
- **Гибкий ввод**: Шаблоны из командной строки или файлов шаблонов
- **Потоковая обработка**: Построчная обработка или потоковый режим
- **Управление выводом**: Различные форматы вывода и опции кодировки
- **Цепочки шаблонов**: Последовательное применение нескольких шаблонов

## Компиляция

Этот проект был перенесен с C++Builder на **Visual Studio 2022**.

### Требования
- Visual Studio 2022 (набор инструментов v143)
- Windows SDK 10.0
- Поддержка стандарта C++17

### Инструкции по сборке
```bash
# Откройте командную строку разработчика для VS 2022
cd rx_vs2022
msbuild rxsnr.sln /p:Configuration=Release /p:Platform=x64
```

Или откройте `rxsnr.sln` в Visual Studio 2022 и соберите решение.

## Использование

### Синтаксис командной строки

```bash
rxsnr [-опции] /поиск/замена[/число] <вход> <выход>
rxsnr [-опции] /поиск/замена/число/вход/[+]выход
rxsnr [-опции] правила.txt <вход> <выход>
rxsnr [-опции] правила.txt вход [+]выход
```

### Опции

- `-h` - Показать экран справки
- `-H` - Показать полную справку с синтаксисом регулярных выражений
- `-d` - Режим отладки (показать информацию об обработке)
- `-b` - Остановиться после первого совпадения в списке шаблонов
- `-u` - Вывод в Unicode (UTF-16)
- `-8` - Вывод в UTF-8
- `-s` - Потоковый режим (игнорировать окончания строк ввода)

### Примеры

#### Базовый поиск и замена
```bash
# Заменить "привет" на "здравствуй" в файле.txt
rxsnr "/привет/здравствуй/" input.txt output.txt

# Замена с учетом регистра и отладочным выводом
rxsnr -d "/Привет/Здравствуй/" input.txt output.txt
```

#### Использование файлов шаблонов
```bash
# Создать файл шаблонов
echo "/старый/новый/" > patterns.txt
echo "/foo/bar/" >> patterns.txt

# Применить шаблоны из файла
rxsnr patterns.txt input.txt output.txt
```

#### Продвинутые шаблоны
```bash
# Заменить цифры на "ЧИСЛО" + цифра
rxsnr "/([0-9])/ЧИСЛО\\1/" input.txt output.txt

# Заменить email адреса
rxsnr "/([a-zA-Z0-9._%-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,})/EMAIL/" input.txt output.txt
```

#### Опции вывода
```bash
# Добавить к существующему файлу
rxsnr "/шаблон/замена/" input.txt +output.txt

# Вывод в UTF-8
rxsnr -8 "/шаблон/замена/" input.txt output.txt

# Вывод в UTF-16
rxsnr -u "/шаблон/замена/" input.txt output.txt
```

### Формат файла шаблонов

Файлы шаблонов содержат одно или несколько правил поиска/замены:
```
/шаблон_поиска/текст_замены/[опции]
```

Строки, начинающиеся с `;`, рассматриваются как комментарии.

## Синтаксис регулярных выражений

### Шаблоны поиска

| Шаблон | Описание |
|--------|----------|
| `^` | Совпадение начала строки |
| `$` | Совпадение конца строки |
| `.` | Совпадение любого символа |
| `*` | Ноль или более вхождений |
| `+` | Одно или более вхождений |
| `?` | Опциональный (ноль или одно вхождение) |
| `[abc]` | Совпадение любого символа в скобках |
| `[^abc]` | Совпадение любого символа НЕ в скобках |
| `[a-z]` | Диапазон символов |
| `(шаблон)` | Группа (поддерживается до 16 групп) |
| `шаблон1|шаблон2` | Альтернатива (ИЛИ) |
| `\\` | Экранирование специальных символов |

### Шаблоны замены

| Шаблон | Описание |
|--------|----------|
| `\\0` до `\\9` | Вызов совпавших групп 1-10 |
| `\\A` до `\\F` | Вызов совпавших групп 11-16 |
| `&` | Весь совпавший шаблон |
| `\\n`, `\\t`, `\\r` | Новая строка, табуляция, возврат каретки |
| `\\\\` | Литеральный обратный слеш |

## Поддержка кодировок файлов

rxsnr автоматически определяет кодировку входного файла:
- **UTF-16**: Файлы, начинающиеся с BOM (0xFF 0xFE)
- **UTF-8**: Файлы, начинающиеся с BOM (0xEF 0xBB 0xBF)
- **ASCII/ANSI**: Кодировка по умолчанию для других файлов

Кодировка вывода может быть указана с помощью опций командной строки.

## Структура проекта

```
rx_vs2022/
├── rxsearch.cpp      # Исходный код основного приложения
├── RegExpClass.h     # Заголовочный файл движка регулярных выражений
├── RegExpClass.cpp   # Реализация движка регулярных выражений
├── rxsnr.vcxproj     # Файл проекта Visual Studio
├── rxsnr.sln         # Файл решения Visual Studio
└── README.md         # Этот файл
```

## Примечания по миграции

Этот проект был успешно перенесен с C++Builder на Visual Studio 2022. Ключевые изменения включают:

- Замена заголовочных файлов и прагм, специфичных для C++Builder
- Обновление разбора аргументов для использования стандартных `argc`/`argv`
- Добавление правильной конфигурации проекта Visual Studio
- Сохранение оригинального алгоритма и функциональности

## Лицензия

Copyright (c) 2014 Dmitry Orlov (dimorlus@gmail.com)

---

# srtmp - Template-Based Search and Replace

**srtmp** is a template-based text processing utility inspired by sed/awk. It processes input files line-by-line using multiple template rules, applying replacements based on regular expression patterns.

## Features

- **Template-Based Processing**: Define multiple search/replace rules in a single template file
- **Line-by-Line Processing**: Each line is tested against all templates sequentially
- **First-Match Principle**: Only the first matching template is applied to each line
- **Multi-Line Replacements**: Single matching lines can be replaced with multiple output lines
- **Header/Footer Support**: Add static header and footer sections to output
- **Regular Expression Engine**: Uses the same powerful RegExp engine as rxsnr

## Compilation

This project has been migrated from C++Builder to **Visual Studio 2022**.

### Requirements
- Visual Studio 2022 (v143 toolset)
- Windows SDK 10.0
- C++17 standard support

### Build Instructions
```bash
# Open Developer Command Prompt for VS 2022
cd rx_vs2022
msbuild srtmp.sln /p:Configuration=Release /p:Platform=x64
```

Or open `srtmp.sln` in Visual Studio 2022 and build the solution.

## Usage

### Command Line Syntax

```bash
srtmp [-t] -f template.txt input.txt [+]output.txt
```

### Options

- `-f` - Specify template file (required)
- `-t` - Do not replace tabs with spaces (optional)
- `-h` - Show help screen
- `-H` - Show full help with regular expression syntax
- `+output.txt` - Append to output file instead of overwriting

### Template File Format

The template file has the following structure:

```
Optional header section
(copied once to the beginning of output)

/regexp1/replacement1/
/regexp2/replacement2
line2
line3/
/regexp3/replacement3/

Optional footer section
(copied once to the end of output)
```

**Format Rules:**
- Templates are delimited by `/` characters
- A template consists of:
  - `/regexp/` - Regular expression pattern to match
  - `replacement` - Replacement text (can span multiple lines)
  - `/` - Optional closing delimiter
- The replacement section can contain multiple lines
- Lines before the first template become the header
- Lines after the last template become the footer
- Each input line is tested against templates in order
- Only the first matching template is applied to each line

### Template Examples

#### Simple Replacement
```
/error/ERROR/
/warning/WARNING/
```

#### Multi-Line Replacement
```
/function (\w+)\(/Function: \1
Parameters:
/
```

#### With Header and Footer
```
HTML Report
<html><body>

/error/(.*)//<div class="error">\1</div>/
/warning/(.*)//<div class="warn">\1</div>/

</body></html>
```

## Processing Algorithm

1. **Load template file**: Parse header, templates, and footer sections
2. **Process input**: Read input file line by line
3. **Match templates**: For each line, test against all templates in order
4. **Apply replacement**: When a match is found:
   - Apply the replacement pattern
   - Output the result (possibly multiple lines)
   - Skip remaining templates for this line
5. **Output**: Write header, processed lines, and footer

## Examples

### Example 1: Log File Processing
```bash
# Create template file
cat > log_template.txt << 'EOF'
Log Processing Report
==================

/ERROR/(.*)/[!] ERROR: \1/
/WARN/(.*)/[*] Warning: \1/
/INFO/(.*)/[i] Info: \1/

==================
End of Report
EOF

# Process log file
srtmp -f log_template.txt server.log report.txt
```

### Example 2: Code Comment Conversion
```bash
# Convert C++ comments to Python comments
cat > cpp2py.txt << 'EOF'
/\/\/(.*)/# \1/
/\/\*(.*)$/# \1/
/(.*)\*\//# \1/
EOF

srtmp -f cpp2py.txt code.cpp code.py
```

### Example 3: Data Transformation
```bash
# Transform CSV to formatted text
cat > csv_format.txt << 'EOF'
Data Report
-----------

/(\w+),(\w+),(\d+)/Name: \1 \2, Age: \3/

-----------
EOF

srtmp -f csv_format.txt data.csv report.txt
```

## Regular Expression Syntax

srtmp uses the same regular expression engine as rxsnr. See the rxsnr section above for complete syntax reference.

## Differences from rxsnr

| Feature | rxsnr | srtmp |
|---------|-------|-------|
| Input method | Command line or file | Template file only |
| Processing | All patterns to all lines | First match only per line |
| Multi-line replacement | No | Yes |
| Header/Footer | No | Yes |
| Tab handling | N/A | Optional (-t flag) |
| Output encoding | Multiple options | Standard output |

## Project Structure

```
rx_vs2022/
├── srtmp.cpp         # Main application source
├── RegExpClass.h     # Regular expression engine header
├── RegExpClass.cpp   # Regular expression engine implementation
├── srtmp.vcxproj     # Visual Studio project file
├── srtmp.sln         # Visual Studio solution file
└── srtmp.txt         # Template format documentation
```

## Use Cases

- **Log file processing**: Convert raw logs to formatted reports
- **Code transformation**: Convert between code styles or languages
- **Data formatting**: Transform structured data into readable formats
- **Report generation**: Add headers, footers, and formatting to data
- **Text cleanup**: Apply multiple cleanup rules in a single pass
- **Template expansion**: Generate output from template-based input

## License

Copyright (c) 2017, 2019 Dmitry Orlov (dimorlus@gmail.com)

---

# srtmp - Шаблонный Поиск и Замена

**srtmp** — утилита для обработки текста на основе шаблонов, вдохновлённая sed/awk. Обрабатывает входные файлы построчно, используя множественные правила шаблонов и применяя замены на основе регулярных выражений.

## Возможности

- **Обработка на основе шаблонов**: Определение множественных правил поиска/замены в одном файле шаблонов
- **Построчная обработка**: Каждая строка проверяется последовательно на соответствие всем шаблонам
- **Принцип первого совпадения**: К каждой строке применяется только первый совпавший шаблон
- **Многострочные замены**: Одна совпавшая строка может быть заменена на несколько строк вывода
- **Поддержка заголовка/подвала**: Добавление статических секций заголовка и подвала к выводу
- **Движок регулярных выражений**: Использует тот же мощный движок RegExp, что и rxsnr

## Компиляция

Этот проект был перенесен с C++Builder на **Visual Studio 2022**.

### Требования
- Visual Studio 2022 (набор инструментов v143)
- Windows SDK 10.0
- Поддержка стандарта C++17

### Инструкции по сборке
```bash
# Откройте командную строку разработчика для VS 2022
cd rx_vs2022
msbuild srtmp.sln /p:Configuration=Release /p:Platform=x64
```

Или откройте `srtmp.sln` в Visual Studio 2022 и соберите решение.

## Использование

### Синтаксис командной строки

```bash
srtmp [-t] -f шаблон.txt входной.txt [+]выходной.txt
```

### Опции

- `-f` - Указать файл шаблонов (обязательно)
- `-t` - Не заменять табуляции на пробелы (опционально)
- `-h` - Показать экран справки
- `-H` - Показать полную справку с синтаксисом регулярных выражений
- `+выходной.txt` - Добавить к выходному файлу вместо перезаписи

### Формат файла шаблонов

Файл шаблонов имеет следующую структуру:

```
Опциональная секция заголовка
(копируется один раз в начало вывода)

/регвыр1/замена1/
/регвыр2/замена2
строка2
строка3/
/регвыр3/замена3/

Опциональная секция подвала
(копируется один раз в конец вывода)
```

**Правила формата:**
- Шаблоны разделяются символами `/`
- Шаблон состоит из:
  - `/регвыр/` - Шаблон регулярного выражения для сопоставления
  - `замена` - Текст замены (может занимать несколько строк)
  - `/` - Опциональный закрывающий разделитель
- Секция замены может содержать несколько строк
- Строки перед первым шаблоном становятся заголовком
- Строки после последнего шаблона становятся подвалом
- Каждая входная строка проверяется на соответствие шаблонам по порядку
- К каждой строке применяется только первый совпавший шаблон

### Примеры шаблонов

#### Простая замена
```
/ошибка/ОШИБКА/
/предупреждение/ПРЕДУПРЕЖДЕНИЕ/
```

#### Многострочная замена
```
/функция (\w+)\(/Функция: \1
Параметры:
/
```

#### С заголовком и подвалом
```
HTML Отчёт
<html><body>

/ошибка/(.*)//<div class="error">\1</div>/
/предупреждение/(.*)//<div class="warn">\1</div>/

</body></html>
```

## Алгоритм обработки

1. **Загрузка файла шаблонов**: Разбор секций заголовка, шаблонов и подвала
2. **Обработка входа**: Чтение входного файла построчно
3. **Сопоставление шаблонов**: Для каждой строки проверка на соответствие всем шаблонам по порядку
4. **Применение замены**: При обнаружении совпадения:
   - Применение шаблона замены
   - Вывод результата (возможно, несколько строк)
   - Пропуск остальных шаблонов для этой строки
5. **Вывод**: Запись заголовка, обработанных строк и подвала

## Примеры

### Пример 1: Обработка лог-файла
```bash
# Создать файл шаблонов
cat > log_template.txt << 'EOF'
Отчёт обработки лога
====================

/ERROR/(.*)/[!] ОШИБКА: \1/
/WARN/(.*)/[*] Предупреждение: \1/
/INFO/(.*)/[i] Информация: \1/

====================
Конец отчёта
EOF

# Обработать лог-файл
srtmp -f log_template.txt server.log report.txt
```

### Пример 2: Преобразование комментариев в коде
```bash
# Конвертировать комментарии C++ в Python
cat > cpp2py.txt << 'EOF'
/\/\/(.*)/# \1/
/\/\*(.*)$/# \1/
/(.*)\*\//# \1/
EOF

srtmp -f cpp2py.txt code.cpp code.py
```

### Пример 3: Трансформация данных
```bash
# Преобразовать CSV в форматированный текст
cat > csv_format.txt << 'EOF'
Отчёт по данным
---------------

/(\w+),(\w+),(\d+)/Имя: \1 \2, Возраст: \3/

---------------
EOF

srtmp -f csv_format.txt data.csv report.txt
```

## Синтаксис регулярных выражений

srtmp использует тот же движок регулярных выражений, что и rxsnr. См. секцию rxsnr выше для полного справочника по синтаксису.

## Отличия от rxsnr

| Функция | rxsnr | srtmp |
|---------|-------|-------|
| Метод ввода | Командная строка или файл | Только файл шаблонов |
| Обработка | Все шаблоны ко всем строкам | Только первое совпадение на строку |
| Многострочная замена | Нет | Да |
| Заголовок/Подвал | Нет | Да |
| Обработка табуляции | Н/Д | Опционально (флаг -t) |
| Кодировка вывода | Множественные опции | Стандартный вывод |

## Структура проекта

```
rx_vs2022/
├── srtmp.cpp         # Исходный код основного приложения
├── RegExpClass.h     # Заголовочный файл движка регулярных выражений
├── RegExpClass.cpp   # Реализация движка регулярных выражений
├── srtmp.vcxproj     # Файл проекта Visual Studio
├── srtmp.sln         # Файл решения Visual Studio
└── srtmp.txt         # Документация по формату шаблонов
```

## Варианты использования

- **Обработка лог-файлов**: Преобразование сырых логов в форматированные отчёты
- **Трансформация кода**: Преобразование между стилями кода или языками
- **Форматирование данных**: Преобразование структурированных данных в читаемые форматы
- **Генерация отчётов**: Добавление заголовков, подвалов и форматирования к данным
- **Очистка текста**: Применение множественных правил очистки за один проход
- **Развёртывание шаблонов**: Генерация вывода из входных данных на основе шаблонов

## Лицензия

Copyright (c) 2017, 2019 Dmitry Orlov (dimorlus@gmail.com)