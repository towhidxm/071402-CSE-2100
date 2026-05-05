package org.example.registryviewer.registry;

/**
 * Wraps Win32/registry failures so callers do not depend on JNA exception types.
 */
public class RegistryAccessException extends Exception {

    public RegistryAccessException(String message) {
        super(message);
    }

    public RegistryAccessException(String message, Throwable cause) {
        super(message, cause);
    }
}

