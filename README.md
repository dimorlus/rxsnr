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