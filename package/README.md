# Packager

Packager is a command-line tool designed to package resource files and manage binary file caches for a given project directory.

## Usage

```
./Packager <project_root_dir> [OPTIONAL]...
```

### Arguments

- `<project_root_dir>`  
  The root directory of your project, which should contain the following subdirectories:  
  - `./Vendor/`
  - `./Source/Textures/`
  - `./Source/Shaders/`

### Options

- `--clean`  
  Clears the binary file cache located in `<bin>/Cache`.

- `--only-copy=<...>`  
  Copies specified resources and exits without further processing. Supported values:   
  - `shaders` – Copies only shader files  
  - `textures` – Copies only texture files  
  - `dlls` – Copies only DLL files  
  - `all` – Copies all of the above  

## Description

1. **Packages all resource files**  
   - Resources from `<project_root_dir>/Source/<resource>` are packaged into `<bin>/<resource>`.
     
2. **Copies necessary DLL files**  
   - Essential DLL dependencies are copied into the `<bin>` directory for execution.
