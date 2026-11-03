# Leitor-exibidor de bytecode

O leitor exibidor deve ser capaz de ler uma arquivo ponto class e apresentar as estruturas nele contidas, de forma similar ao visualizador jclasslib. Os índices para slots no pool de constantes devem ser incluídos, assim como a constante representada, de forma similar ao que ocorre no jclasslib. Mnemônicos dos bytecodes devem ser apresentados e os seus parâmetros (argumentos) que apontam para o pool de constantes deve indicar os valores das constantes a que se referem. 

As versões usadas do Java são 2, 6 e 8.

Este projeto deve ser compilado com GCC usando `-std=c++11`.

## Dependencias

Este projeto usa CMake para o build. Dependências necessárias (resumido):

- CMake (>= 3.16 recomendado; use a versão estável mais recente)
- Um compilador C++ moderno (GCC ou MSVC no Windows)
- Ninja ou Make (gerador de build, opcional)
<!-- - Doxygen (para gerar a documentação em `doxygen/`) -->

As instruções abaixo mostram como instalar o cmake nas plataformas mais comuns.

### Linux (Debian/Ubuntu)

<!-- - **doxygen**
```bash
sudo apt update
sudo apt install -y build-essential doxygen
``` -->

- **CMake**

```bash
sudo apt update
sudo apt install -y cmake
```

### Windows (opções de instaladores simples)

PowerShell (Admin):

- **chocolatey**
```powershell
choco install -y cmake --installargs 'ADD_CMAKE_TO_PATH=System' doxygen
```

- **winget**
```powershell
winget install --id Kitware.CMake -e --source winget
winget install --id Doxygen.Doxygen -e --source winget
```

<!-- ### Observações
- Certifique-se que `doxygen` esteja no `PATH` para que `./doxygen/doxygen-build.sh` ou o target CMake `doc_doxygen` possam ser executados. -->

### Início rápido (local)

```bash
mkdir -p build && cd build
cmake .. -DBUILD_TESTING=ON
cmake --build .
```

Para executar testes:

```
ctest --output-on-failure
```

O Projeto pode ser executado por meio da execução do binário principal do programa,

**linux**
```
./build/src/bytecode_viewer_cli <class-file>
```

**windows**
```
.\build\Debug\bytecode_viewer_cli.exe <class-file>
```

## Estrutura do diretório

O diretório é organizado da seguinte forma:

```
.
├── exemplos/                       # arquivos .class de teste
├── include/
│   └── bytecode_viewer/
│       ├── Common.hpp              # Definições de tipos (u1, u2, u4) 
                                    e macros Big-Endian
│       ├── ConstantPool.hpp        # Structs das tags do Pool de Constantes
│       ├── ClassStructures.hpp     # Structs de Methods, Fields e Attributes (Code)
│       ├── ClassReader.hpp         # Classe responsável por ler o arquivo
                                    binário
│       └── BytecodeDecoder.hpp     # Mapeamento e decodificação dos mnemônicos
├── src/
│   ├── main.cpp                    # Entrada do CLI, processa argv e chama a 
                                    exibição
│   └── bytecode_viewer/
│       ├── ClassReader.cpp         # Implementação dos métodos de leitura de 
                                    bytes
│       ├── ConstantPool.cpp        # Lógica de resolução e exibição de 
                                    constantes (#1 = ...)
│       └── BytecodeDecoder.cpp     # Tabela de mnemônicos e lógica de avanço 
                                    do PC
├── tests/                          # Testes unitários (opcional)
├── CMakeLists.txt
└── README.md                       # Instruções de compilação e uso do CLI
```

O output da leitura de um arquivo `.class` é como na imagem abaixo: 

```
General Information:
  Magic:                   0xCAFEBABE
  Minor version:           0
  Major version:           52 [8]
  Constant pool count:     15
  Access flags:            0x0021 [public, super]
  This class:              cp_info #2 <Hello>
  Super class:             cp_info #4 <java/lang/Object>
  Interfaces count:        0
  Fields count:            0
  Methods count:           1
  Attributes count:        1

Constant Pool:
  #1 = Utf8               "Hello"
  #2 = Class              #1 // Hello
  #3 = Utf8               "java/lang/Object"
  #4 = Class              #3 // java/lang/Object
  #5 = Utf8               "main"
  #6 = Utf8               "([Ljava/lang/String;)V"
  #7 = Utf8               "java/lang/System"
  #8 = Class              #7 // java/lang/System
  #9 = Utf8               "out"
  #10 = Utf8              "Ljava/io/PrintStream;"
  #11 = NameAndType       #9:#10 // out:Ljava/io/PrintStream;
  #12 = Fieldref          #8.#11 // java/lang/System.out:Ljava/io/PrintStream;

Methods:
  Method Name: main
  Descriptor:  ([Ljava/lang/String;)V
  Code Attribute:
    0: getstatic       #12 // Field java/lang/System.out:Ljava/io/PrintStream;
    3: ldc             #13 // String "Hello, World!"
    5: invokevirtual   #14 // Method java/io/PrintStream.println:(Ljava/lang/String;)V
    8: return
```

## Testes

Os testes automatizados atuais validam a biblioteca base do projeto. A cobertura de leitura e exibição de bytecode pode ser ampliada conforme o interpretador evoluir.

### jclasslib

Para usar o jclasslib, baixe o binário da distribuição oficial [link](https://github.com/ingokegel/jclasslib/releases/tag/7.1), por exemplo, .tar para linux, e execute-o pelo comando `./jclasslib/bin/jclasslib` para usar a interface gráfica. 