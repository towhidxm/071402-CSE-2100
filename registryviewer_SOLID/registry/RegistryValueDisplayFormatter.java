package org.example.registryviewer.registry;

import org.example.registryviewer.domain.RegistryValueRecord;

import java.util.List;
import java.util.Map;

/**
 * Converts raw value map entries into display rows.
 */
public interface RegistryValueDisplayFormatter {

    List<RegistryValueRecord> format(Map<String, Object> valuesByName);
}

