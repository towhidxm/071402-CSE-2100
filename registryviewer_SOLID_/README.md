# Registry Viewer (SOLID)

## Overview
This is a Java (Swing) desktop application that lets you browse Windows Registry keys and values using a read-only backend implemented with JNA.

## Project Structure
```
registryviewer_SOLID/
  Main.java
  app/
    RegistryController.java
  config/
    AppConstants.java
  domain/
    RootHive.java
    RegistryKeyNode.java
    RegistryValueRecord.java
  registry/
    RegistryReadService.java
    JnaRegistryReadService.java
    DefaultRegistryValueDisplayFormatter.java
  ui/
    RegistryViewerFrame.java
    RegistryView.java
    RegistryTreeView.java
```

## Prerequisites
- Windows
- Java Development Kit (JDK) 11+
- JNA (Java Native Access) jars on the classpath

## Setup (JNA)
Download the JNA jars and place them in `lib/` at the repo root:
- `lib/jna-*.jar`
- `lib/jna-platform-*.jar`

The `Makefile` expects these jars via glob patterns.

## Build
From the repo root:
```sh
make build
```

Output classes go to `build/`.

## Run
```sh
make run
```

The application entry point is:
`org.example.registryviewer.Main`

## Clean
```sh
make clean
```

