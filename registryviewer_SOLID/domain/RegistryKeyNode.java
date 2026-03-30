package org.example.registryviewer.domain;

import com.sun.jna.platform.win32.WinReg;

import java.util.Objects;

/**
 * User object for tree nodes: either a real key or a lazy-load placeholder.
 */
public final class RegistryKeyNode {

    private final String displayName;
    private final String fullPath;
    private final WinReg.HKEY hkey;
    private final boolean placeholder;

    private RegistryKeyNode(String displayName, String fullPath, WinReg.HKEY hkey, boolean placeholder) {
        this.displayName = displayName;
        this.fullPath = fullPath;
        this.hkey = hkey;
        this.placeholder = placeholder;
    }

    public static RegistryKeyNode placeholderChild() {
        return new RegistryKeyNode("Loading…", "", null, true);
    }

    public static RegistryKeyNode realKey(String displayName, String fullPath, WinReg.HKEY hkey) {
        return new RegistryKeyNode(displayName, fullPath, hkey, false);
    }

    public String getDisplayName() {
        return displayName;
    }

    public String getFullPath() {
        return fullPath;
    }

    public WinReg.HKEY getHkey() {
        return hkey;
    }

    public boolean isPlaceholder() {
        return placeholder;
    }

    public boolean isLoadingPlaceholder() {
        return placeholder && displayName.startsWith("Loading");
    }

    @Override
    public String toString() {
        return displayName;
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (!(o instanceof RegistryKeyNode that)) {
            return false;
        }
        return placeholder == that.placeholder
                && Objects.equals(displayName, that.displayName)
                && Objects.equals(fullPath, that.fullPath)
                && Objects.equals(hkey, that.hkey);
    }

    @Override
    public int hashCode() {
        return Objects.hash(displayName, fullPath, hkey, placeholder);
    }
}

