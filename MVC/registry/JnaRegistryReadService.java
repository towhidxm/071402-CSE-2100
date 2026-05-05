package org.example.registryviewer.registry;

import com.sun.jna.platform.win32.Advapi32Util;
import com.sun.jna.platform.win32.Win32Exception;
import com.sun.jna.platform.win32.WinNT;
import com.sun.jna.platform.win32.WinReg.HKEY;
import com.sun.jna.platform.win32.WinReg.HKEYByReference;
import org.example.registryviewer.domain.RootHive;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * Read-only registry facade implemented with JNA / Advapi32.
 */
public final class JnaRegistryReadService implements RegistryReadService {

    private static final List<HKEY> PREDEFINED_ROOTS = Collections.unmodifiableList(Arrays.asList(
            RootHive.HKEY_CLASSES_ROOT.getHkey(),
            RootHive.HKEY_CURRENT_USER.getHkey(),
            RootHive.HKEY_LOCAL_MACHINE.getHkey(),
            RootHive.HKEY_USERS.getHkey(),
            RootHive.HKEY_CURRENT_CONFIG.getHkey()
    ));

    @Override
    public HKEY openChildReadOnly(HKEY parent, String subkeyName) throws RegistryAccessException {
        try {
            HKEYByReference ref = Advapi32Util.registryGetKey(parent, subkeyName, WinNT.KEY_READ);
            return ref.getValue();
        } catch (Win32Exception e) {
            throw new RegistryAccessException("Failed to open subkey: " + subkeyName, e);
        }
    }

    @Override
    public List<String> listSubkeyNames(HKEY key) throws RegistryAccessException {
        try {
            String[] keys = Advapi32Util.registryGetKeys(key);
            return List.of(keys);
        } catch (Win32Exception e) {
            throw new RegistryAccessException("Failed to enumerate subkeys", e);
        }
    }

    @Override
    @SuppressWarnings("unchecked")
    public Map<String, Object> listValues(HKEY key) throws RegistryAccessException {
        try {
            // JNA returns a TreeMap<String, Object> with values typed by REG_*.
            return Advapi32Util.registryGetValues(key);
        } catch (Win32Exception e) {
            throw new RegistryAccessException("Failed to enumerate values", e);
        }
    }

    @Override
    public boolean isPredefinedRoot(HKEY key) {
        if (key == null) {
            return false;
        }
        return PREDEFINED_ROOTS.stream().anyMatch(key::equals);
    }

    @Override
    public void releaseKey(HKEY key) throws RegistryAccessException {
        if (key == null || isPredefinedRoot(key)) {
            return;
        }
        try {
            Advapi32Util.registryCloseKey(key);
        } catch (Win32Exception e) {
            throw new RegistryAccessException("Failed to close key", e);
        }
    }
}

