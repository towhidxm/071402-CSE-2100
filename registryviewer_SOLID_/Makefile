SRC_DIR := registryviewer_SOLID
BIN_DIR := build
LIB_DIR := lib

# Windows (PowerShell) helper to list all Java sources recursively.
SOURCES := $(shell powershell -NoProfile -Command "Get-ChildItem -Path '$(SRC_DIR)' -Recurse -Filter '*.java' | ForEach-Object { $$_.FullName }")

# Pick JNA jars from `lib/` (you download these yourself).
JNA_JAR := $(firstword $(wildcard $(LIB_DIR)/jna*.jar))
JNA_PLATFORM_JAR := $(firstword $(wildcard $(LIB_DIR)/jna-platform*.jar))

ifeq ($(strip $(JNA_PLATFORM_JAR)),)
$(error Missing JNA jars. Place `lib/jna-*.jar` and `lib/jna-platform-*.jar` in the repo root.)
endif

ifeq ($(strip $(JNA_JAR)),)
$(error Missing `lib/jna-*.jar`. Place the JNA jars in the repo root under `lib/`.)
endif

CLASSPATH_COMPILE := $(BIN_DIR);$(JNA_JAR);$(JNA_PLATFORM_JAR)
CLASSPATH_RUN := $(BIN_DIR);$(JNA_JAR);$(JNA_PLATFORM_JAR)

.PHONY: all build run clean

all: build

build:
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(BIN_DIR)' | Out-Null"
	javac -encoding UTF-8 -d "$(BIN_DIR)" -cp "$(CLASSPATH_COMPILE)" $(SOURCES)

run: build
	java -cp "$(CLASSPATH_RUN)" org.example.registryviewer.Main

clean:
	@powershell -NoProfile -Command "if (Test-Path '$(BIN_DIR)') { Remove-Item -Recurse -Force '$(BIN_DIR)' }"

