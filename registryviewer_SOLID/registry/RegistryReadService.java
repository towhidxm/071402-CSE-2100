package org.example.registryviewer.registry;

import com.sun.jna.platform.win32.WinReg.HKEY;

import java.util.List;
import java.util.Map;

/**
 * Read-only registry access (Dependency Inversion: UI/controller depend on this abstraction).
 */
public interface RegistryReadService {

    /**
     * Opens a direct subkey of {@code parent} for read access.
     * Caller must close it via {@link #releaseKey(HKEY)} when no longer needed
     * (predefined roots are handled as a no-op).
     */
    HKEY openChildReadOnly(HKEY parent, String subkeyName) throws RegistryAccessException;

    List<String> listSubkeyNames(HKEY key) throws RegistryAccessException;

    Map<String, Object> listValues(HKEY key) throws RegistryAccessException;

    boolean isPredefinedRoot(HKEY key);

    /**
     * Closes handles opened via {@link #openChildReadOnly}; no-op for predefined roots.
     */
    void releaseKey(HKEY key) throws RegistryAccessException;
}

