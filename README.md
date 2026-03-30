# Registry Viewer (SOLID Refactor)

## Overview
This project is a Java (Swing) refactor of a procedural registry-viewer style application.  
The codebase is organized with object-oriented design and SOLID principles so it stays modular, extensible, and maintainable.

## Prompt Used (as requested)
```text
Prompt: Refactoring a C Project into a Java-Based SOLID Object-Oriented Design

Refactor an existing C project into a Java-based Object-Oriented Design (OOD) system by applying SOLID principles. Follow the steps below:

Convert the procedural C code into Java, following object-oriented design practices (classes, objects, encapsulation, inheritance, polymorphism).
While refactoring, apply the following SOLID principles:
SRP (Single Responsibility Principle): Each class should have only one responsibility.
OCP (Open/Closed Principle): Design classes so they are open for extension but closed for modification.
LSP (Liskov Substitution Principle): Subclasses should be replaceable for their base classes without affecting correctness.
ISP (Interface Segregation Principle): Use multiple small, specific interfaces instead of large, general ones.
DIP (Dependency Inversion Principle): Depend on abstractions (interfaces), not concrete implementations.
Organize the project into proper packages (e.g., model, service, repository, interface).
Ensure the code is modular, scalable, and maintainable.
Add proper JavaDoc comments and a README.md explaining:
Project structure
How SOLID principles are applied
How to run the project
```

## Project Structure
```text
registryviewer_SOLID/
  Main.java
  app/
    RegistryController.java
    RegistryTreeCoordinator.java
    DefaultRegistryTreeCoordinator.java
    RegistryValueCoordinator.java
    DefaultRegistryValueCoordinator.java
  config/
    AppConstants.java
  domain/                        # model layer
    RootHive.java
    RegistryKeyNode.java
    RegistryValueRecord.java
  registry/                      # service/repository boundary for registry I/O
    RegistryReadService.java
    JnaRegistryReadService.java
    RegistryValueDisplayFormatter.java
    DefaultRegistryValueDisplayFormatter.java
    RegistryAccessException.java
  ui/                            # interface/presentation layer
    RegistryView.java
    RegistryTreeView.java
    RegistryViewerFrame.java
```

## How SOLID Is Applied
- **SRP:** UI layout, controller orchestration, tree-loading logic, and value-formatting logic are separated into focused classes.
- **OCP:** `RegistryReadService`, `RegistryValueDisplayFormatter`, `RegistryTreeCoordinator`, and `RegistryValueCoordinator` are extension points.
- **LSP:** Implementations (`JnaRegistryReadService`, default coordinators/formatter) can replace their interfaces without caller changes.
- **ISP:** Small, purpose-specific interfaces (`RegistryView`, `RegistryTreeView`, `RegistryReadService`) avoid one large "god" contract.
- **DIP:** High-level flow depends on abstractions and receives concrete implementations from composition root (`Main`).

## JavaDoc Status
Core classes and interfaces contain JavaDoc comments describing responsibilities and key methods/constructors.

## Prerequisites
- Windows
- Java Development Kit (JDK) 11+
- GNU Make
- JNA jars in `lib/`

## Setup
Place these jars in the repo-root `lib/` folder:
- `lib/jna-*.jar`
- `lib/jna-platform-*.jar`

## Build
```sh
make build
```

## Run
```sh
make run
```
Entry point: `org.example.registryviewer.Main`

## Clean
```sh
make clean
```

