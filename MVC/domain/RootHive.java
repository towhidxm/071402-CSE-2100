package org.example.registryviewer.domain;

import com.sun.jna.platform.win32.WinReg;

/**
 * The five standard registry roots shown at the top of the tree.
 */
public enum RootHive {
    HKEY_CLASSES_ROOT(WinReg.HKEY_CLASSES_ROOT, "HKEY_CLASSES_ROOT"),
    HKEY_CURRENT_USER(WinReg.HKEY_CURRENT_USER, "HKEY_CURRENT_USER"),
    HKEY_LOCAL_MACHINE(WinReg.HKEY_LOCAL_MACHINE, "HKEY_LOCAL_MACHINE"),
    HKEY_USERS(WinReg.HKEY_USERS, "HKEY_USERS"),
    HKEY_CURRENT_CONFIG(WinReg.HKEY_CURRENT_CONFIG, "HKEY_CURRENT_CONFIG");

    private final WinReg.HKEY hkey;
    private final String displayName;

    RootHive(WinReg.HKEY hkey, String displayName) {
        this.hkey = hkey;
        this.displayName = displayName;
    }

    public WinReg.HKEY getHkey() {
        return hkey;
    }

    public String getDisplayName() {
        return displayName;
    }
}

