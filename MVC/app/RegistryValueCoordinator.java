package org.example.registryviewer.app;

import com.sun.jna.platform.win32.WinReg.HKEY;
import org.example.registryviewer.ui.RegistryView;

/**
 * Loads values for a selected key and renders them in the view.
 */
public interface RegistryValueCoordinator {

    /**
     * Reads values for a key and updates the view's value table/status.
     *
     * @param view active view contract
     * @param hkey selected registry key handle
     */
    void loadValuesForKey(RegistryView view, HKEY hkey);
}
