package org.example.registryviewer.domain;

import java.util.Objects;

/**
 * Immutable row for the value list (Name / Type / Data).
 */
public final class RegistryValueRecord {

    private final String name;
    private final String typeDisplay;
    private final String dataDisplay;

    public RegistryValueRecord(String name, String typeDisplay, String dataDisplay) {
        this.name = Objects.requireNonNullElse(name, "");
        this.typeDisplay = Objects.requireNonNullElse(typeDisplay, "");
        this.dataDisplay = Objects.requireNonNullElse(dataDisplay, "");
    }

    public String getName() {
        return name;
    }

    public String getTypeDisplay() {
        return typeDisplay;
    }

    public String getDataDisplay() {
        return dataDisplay;
    }
}

