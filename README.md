# Multi-Language Obfuscator Tool

This tool obfuscates source code in multiple languages and produces extremely hard-to-read output. Supported languages: C++, Python, JavaScript, Java, C#, PHP.

---

## Build (Windows / Linux, Static)

Open terminal / cmd with MinGW-w64 or g++ installed.

Compile static executable:

```bash
g++ main.cpp -O2 -std=c++17 -static -s -o obf_tool
```

---

## Usage

```bash
./obf_tool obf -i input_file
```

- Input file extensions supported: `.cpp`, `.py`, `.js`, `.java`, `.cs`, `.php`
- Output will automatically be: `<input_file>_obf.<ext>`
- Tool auto-detects the language based on the extension

---

## Examples

### C++

Original `example.cpp`:

```cpp
#include <iostream>
int main() {
    std::cout << "Hello World" << std::endl;
    return 0;
}
```

Obfuscated output `example_obf.cpp`:

```cpp
#include<iostream>
int main(){for(int _=0;_<1;++_)std::cout<<"H"<<'e'<<"l"<<'l'<<"o"<<' '<<"W"<<'o'<<"r"<<'l'<<"d\n";}
```

### Python

Original `example.py`:

```python
print("Hello World")
```

Obfuscated output `example_obf.py`:

```python
for _ in range(1):print(''.join(['H','e','l','l','o',' ','W','o','r','l','d']))
```

### JavaScript

Original `example.js`:

```javascript
console.log("Hello World");
```

Obfuscated output `example_obf.js`:

```javascript
(()=>{for(let _=0;_<1;_++){console['l'+'o'+'g'](['H','e','l','l','o',' ','W','o','r','l','d'].join(''));}})();
```

### Java

Original `Example.java`:

```java
public class Example {
    public static void main(String[] args) {
        System.out.println("Hello World");
    }
}
```

Obfuscated output `Example_obf.java`:

```java
public class Example{public static void main(String[]a){for(int _=0;_<1;++_)System.out.println(String.join("",new String[]{"H","e","l","l","o"," ","W","o","r","l","d"}));}}
```

### C#

Original `Example.cs`:

```csharp
using System;
class Example {
    static void Main() {
        Console.WriteLine("Hello World");
    }
}
```

Obfuscated output `Example_obf.cs`:

```csharp
using System;class Example{static void Main(){for(int _=0;_<1;++_)Console.WriteLine(string.Join("",new string[]{"H","e","l","l","o"," ","W","o","r","l","d"}));}}
```

### PHP

Original `example.php`:

```php
<?php
echo "Hello World";
?>
```

Obfuscated output `example_obf.php`:

```php
<?php for($_=0;$_<1;++$_)echo implode('',array('H','e','l','l','o',' ','W','o','r','l','d')); ?>
```

---

## Notes

- Credit is embedded in every obfuscated file in an encoded form.
- Removing the credit may break the obfuscated file.
- Obfuscation focuses on unreadability over performance.
- Output files are intentionally unreadable.

---

## License / Credit

```
Obfuscator Tool
Author: Hoang Phuc
Encoded Credit: do not remove
```

