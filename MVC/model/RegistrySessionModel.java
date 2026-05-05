package org.example.registryviewer.model;

import com.sun.jna.platform.win32.WinReg.HKEY;

/**
 * MVC model that stores the current application session state.
 */
public final class RegistrySessionModel {

    private HKEY selectedHkey;
    private String selectedPath;
    private boolean autoRefreshEnabled;

    public HKEY getSelectedHkey() {
        return selectedHkey;
    }

    public String getSelectedPath() {
        return selectedPath;
    }

    public boolean isAutoRefreshEnabled() {
        return autoRefreshEnabled;
    }

    public void selectKey(HKEY hkey, String path) {
        this.selectedHkey = hkey;
        this.selectedPath = path;
    }

    public void clearSelection() {
        this.selectedHkey = null;
        this.selectedPath = null;
    }

    public void setAutoRefreshEnabled(boolean autoRefreshEnabled) {
        this.autoRefreshEnabled = autoRefreshEnabled;
    }
}
