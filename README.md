# Windows Registry Viewer — Java SOLID Implementation

Project: Windows Registry Viewer Refactoring 
Course: Advanced Programming Laboratory 
Date: March 2026  
Students: TOWHID AL MAHMUD & ABIR KHAN SIAM

## Table of Contents
- Part A — Project Overview
- Part B — Build & Usage
- Part C — Architecture & Design
- Part D — Refactoring Documentation
- Part E — Appendices

---

## PART A — PROJECT OVERVIEW

### 1. Introduction
This project is a Windows Registry Viewer desktop application built in Java (Swing).  
It was refactored from a procedural mindset into a modular Object-Oriented Design using SOLID principles.

### What This Application Does
- Browse root hives (`HKEY_CLASSES_ROOT`, `HKEY_CURRENT_USER`, etc.)
- Expand registry keys lazily from tree navigation
- Display selected key values in a sortable table format
- Auto-refresh value panel at a configurable interval
- Keep read-only behavior for safe inspection

### 2. Project Version (Current)
This repository currently contains one SOLID-focused Java implementation.

#### Characteristics
- Clean package-based structure
- Interface-driven design (`RegistryReadService`, `RegistryValueDisplayFormatter`, view contracts)
- Controller + coordinator separation for SRP
- JNA integration for Windows registry read access
- JavaDoc-covered key classes and interfaces

### 3. Technology Stack
| Component | Technology | Purpose |
|---|---|---|
| Language | Java (JDK 11+) | Core implementation |
| UI | Swing | Desktop interface |
| Windows Integration | JNA + Advapi32 | Registry API access |
| Build | Makefile + javac | Compilation and run tasks |
| Platform | Windows | Target OS |

### 4. Repository Structure
```text
registryviewer_SOLID/
├── Main.java
├── app/
│   ├── RegistryController.java
│   ├── RegistryTreeCoordinator.java
│   ├── DefaultRegistryTreeCoordinator.java
│   ├── RegistryValueCoordinator.java
│   └── DefaultRegistryValueCoordinator.java
├── config/
│   └── AppConstants.java
├── domain/
│   ├── RootHive.java
│   ├── RegistryKeyNode.java
│   └── RegistryValueRecord.java
├── registry/
│   ├── RegistryReadService.java
│   ├── JnaRegistryReadService.java
│   ├── RegistryValueDisplayFormatter.java
│   ├── DefaultRegistryValueDisplayFormatter.java
│   └── RegistryAccessException.java
└── ui/
    ├── RegistryView.java
    ├── RegistryTreeView.java
    └── RegistryViewerFrame.java
```

---

## PART B — BUILD & USAGE

### 5. Build Requirements
- Windows 10/11
- JDK 11 or later
- JNA jars in `lib/`:
  - `jna-*.jar`
  - `jna-platform-*.jar`
- Optional: `make` command (or run `javac` manually)

### 6. Compilation Instructions
If `make` is available:
```sh
make build
```

If `make` is not available, compile manually from project root:
```powershell
New-Item -ItemType Directory -Force -Path build | Out-Null
$src = Get-ChildItem -Path registryviewer_SOLID -Recurse -Filter *.java | ForEach-Object { $_.FullName }
$jna = (Get-ChildItem lib\jna*.jar | Select-Object -First 1).FullName
$jnap = (Get-ChildItem lib\jna-platform*.jar | Select-Object -First 1).FullName
javac -encoding UTF-8 -d build -cp "build;$jna;$jnap" $src
```

### 7. Running the Application
Using `make`:
```sh
make run
```

Manual run:
```powershell
$jna = (Get-ChildItem lib\jna*.jar | Select-Object -First 1).FullName
$jnap = (Get-ChildItem lib\jna-platform*.jar | Select-Object -First 1).FullName
java -cp "build;$jna;$jnap" org.example.registryviewer.Main
```

### 8. Troubleshooting
- `make` not found: use manual `javac`/`java` commands above.
- JNA jar missing: place both JNA jars inside `lib/`.
- Non-Windows launch: app exits by design (Windows-only).

---

## PART C — ARCHITECTURE & DESIGN

### 9. Layered View
```text
Main (composition root)
   -> app (controller + coordinators)
      -> ui (view contracts + Swing frame)
      -> registry (read service + format strategy)
      -> domain (immutable/value models)
```

### 10. UML Class Diagram

```mermaid
classDiagram
    direction LR

    %% ── Entry Point ──────────────────────────────────────────────
    class Main {
        +main(args: String[]) void
    }

    %% ── Config ───────────────────────────────────────────────────
    class AppConstants {
        +REFRESH_INTERVAL_MS : int
        +ROOT_HIVES : String[]
    }

    %% ── Domain ───────────────────────────────────────────────────
    class RootHive {
        -name : String
        -handle : long
        +getName() String
        +getHandle() long
    }

    class RegistryKeyNode {
        -path : String
        -hive : RootHive
        +getPath() String
        +getHive() RootHive
    }

    class RegistryValueRecord {
        -name : String
        -type : int
        -data : byte[]
        +getName() String
        +getType() int
        +getData() byte[]
    }

    %% ── Registry (interfaces) ────────────────────────────────────
    class RegistryReadService {
        <<interface>>
        +listSubKeys(node: RegistryKeyNode) List~String~
        +listValues(node: RegistryKeyNode) List~RegistryValueRecord~
    }

    class RegistryValueDisplayFormatter {
        <<interface>>
        +format(record: RegistryValueRecord) String
    }

    %% ── Registry (implementations) ───────────────────────────────
    class JnaRegistryReadService {
        +listSubKeys(node: RegistryKeyNode) List~String~
        +listValues(node: RegistryKeyNode) List~RegistryValueRecord~
    }

    class DefaultRegistryValueDisplayFormatter {
        +format(record: RegistryValueRecord) String
    }

    class RegistryAccessException {
        +RegistryAccessException(message: String)
        +RegistryAccessException(message: String, cause: Throwable)
    }

    %% ── UI (interfaces / contracts) ──────────────────────────────
    class RegistryView {
        <<interface>>
        +setStatusMessage(msg: String) void
        +showError(msg: String) void
    }

    class RegistryTreeView {
        <<interface>>
        +setRootHives(hives: List~RootHive~) void
        +appendChildNodes(parent: RegistryKeyNode, children: List~String~) void
    }

    %% ── UI (implementation) ──────────────────────────────────────
    class RegistryViewerFrame {
        -treePanel : JTree
        -valueTable : JTable
        +setRootHives(hives: List~RootHive~) void
        +appendChildNodes(parent: RegistryKeyNode, children: List~String~) void
        +setStatusMessage(msg: String) void
        +showError(msg: String) void
        +setValueRecords(records: List~RegistryValueRecord~) void
    }

    %% ── App (interfaces) ─────────────────────────────────────────
    class RegistryTreeCoordinator {
        <<interface>>
        +loadRootHives() void
        +expandNode(node: RegistryKeyNode) void
    }

    class RegistryValueCoordinator {
        <<interface>>
        +loadValues(node: RegistryKeyNode) void
        +startAutoRefresh(node: RegistryKeyNode) void
        +stopAutoRefresh() void
    }

    %% ── App (implementations) ────────────────────────────────────
    class DefaultRegistryTreeCoordinator {
        -readService : RegistryReadService
        -treeView : RegistryTreeView
        +loadRootHives() void
        +expandNode(node: RegistryKeyNode) void
    }

    class DefaultRegistryValueCoordinator {
        -readService : RegistryReadService
        -formatter : RegistryValueDisplayFormatter
        -view : RegistryView
        +loadValues(node: RegistryKeyNode) void
        +startAutoRefresh(node: RegistryKeyNode) void
        +stopAutoRefresh() void
    }

    class RegistryController {
        -treeCoordinator : RegistryTreeCoordinator
        -valueCoordinator : RegistryValueCoordinator
        +onNodeSelected(node: RegistryKeyNode) void
        +onNodeExpanded(node: RegistryKeyNode) void
        +onRefreshToggled(enabled: boolean) void
    }

    %% ── Relationships ────────────────────────────────────────────

    %% Entry point wires everything
    Main ..> RegistryController : creates
    Main ..> DefaultRegistryTreeCoordinator : creates
    Main ..> DefaultRegistryValueCoordinator : creates
    Main ..> JnaRegistryReadService : creates
    Main ..> DefaultRegistryValueDisplayFormatter : creates
    Main ..> RegistryViewerFrame : creates

    %% Controller depends on coordinator interfaces (DIP)
    RegistryController --> RegistryTreeCoordinator : uses
    RegistryController --> RegistryValueCoordinator : uses

    %% Default coordinators implement interfaces (LSP)
    DefaultRegistryTreeCoordinator ..|> RegistryTreeCoordinator
    DefaultRegistryValueCoordinator ..|> RegistryValueCoordinator

    %% Coordinators depend on service interfaces (DIP)
    DefaultRegistryTreeCoordinator --> RegistryReadService : uses
    DefaultRegistryTreeCoordinator --> RegistryTreeView : uses
    DefaultRegistryValueCoordinator --> RegistryReadService : uses
    DefaultRegistryValueCoordinator --> RegistryValueDisplayFormatter : uses
    DefaultRegistryValueCoordinator --> RegistryView : uses

    %% Concrete implementations satisfy interfaces (LSP)
    JnaRegistryReadService ..|> RegistryReadService
    DefaultRegistryValueDisplayFormatter ..|> RegistryValueDisplayFormatter
    RegistryViewerFrame ..|> RegistryView
    RegistryViewerFrame ..|> RegistryTreeView

    %% Domain model associations
    RegistryKeyNode --> RootHive : belongs to
    RegistryReadService ..> RegistryKeyNode : uses
    RegistryReadService ..> RegistryValueRecord : produces
    RegistryValueDisplayFormatter ..> RegistryValueRecord : formats

    %% Exception thrown by registry service
    JnaRegistryReadService ..> RegistryAccessException : throws
```

### 11. SOLID Principles Analysis
- **SRP:** `RegistryController` handles UI events; tree loading and value loading are delegated to dedicated coordinators.
- **OCP:** New data sources or formatters can be added by implementing interfaces without changing controller flow.
- **LSP:** Any `RegistryReadService` or `RegistryValueDisplayFormatter` implementation can replace defaults.
- **ISP:** Small interfaces (`RegistryView`, `RegistryTreeView`, `RegistryTreeCoordinator`, `RegistryValueCoordinator`) keep contracts focused.
- **DIP:** High-level app logic depends on interfaces, while `Main` wires concrete implementations.

### 12. Package Responsibility Mapping
- `domain`: pure data models
- `registry`: Windows registry data access + value formatting policies
- `ui`: Swing view and view contracts
- `app`: orchestration and application logic
- `config`: static configuration/constants

---

## PART D — REFACTORING DOCUMENTATION

### 13. Prompts Used (Step-by-Step)

#### Step 1 — Initial Conversion Prompt
```text
Convert the existing procedural C project into Java using object-oriented design.
Use classes, objects, encapsulation, inheritance, and polymorphism where appropriate.
```

#### Step 2 — SOLID Refactoring Prompt
```text
Refactor the Java code by applying SOLID principles:
SRP: one responsibility per class,
OCP: open for extension, closed for modification,
LSP: implementations should be substitutable,
ISP: prefer small focused interfaces,
DIP: depend on abstractions, not concrete classes.
```

#### Step 3 — Package Organization Prompt
```text
Organize the project into maintainable packages such as model/domain, service/repository,
application/controller, and interface/ui layers.
```

#### Step 4 — Documentation Prompt
```text
Add proper JavaDoc comments across major classes and interfaces.
Create a README.md describing project structure, SOLID application, and run instructions.
```

### 14. Key Refactoring Changes
- Introduced coordinator abstractions for tree and value workflows.
- Kept UI rendering separate from registry I/O concerns.
- Preserved extension points with interfaces for service and formatter.
- Improved JavaDoc in key contracts and implementations.

### 15. Lessons Learned
- Interface-first design made refactoring safer.
- Splitting responsibilities reduced controller complexity.
- Windows-specific logic is best isolated in registry service implementations.

---

## PART E — APPENDICES

### 16. Dependency Flow Verification
- UI depends on view contracts and app controller only.
- Controller depends on abstractions, not JNA implementation details.
- Registry implementation depends on JNA/Windows API wrappers.
- No circular dependencies across packages.

### 17. Contact
- **Student 1:** TOWHID AL MAHMUD
- **Student 2:** ABIR KHAN SIAM
- **Course:** 0714 02 CSE 2100 — Advanced Programming Laboratory

---
