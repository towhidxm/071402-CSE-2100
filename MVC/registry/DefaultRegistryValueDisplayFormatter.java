package org.example.registryviewer.registry;

import org.example.registryviewer.domain.RegistryValueRecord;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Map;

/**
 * Maps raw JNA values to display columns.
 * Only formatting logic lives here (no UI, no registry calls).
 */
public final class DefaultRegistryValueDisplayFormatter implements RegistryValueDisplayFormatter {

    @Override
    public List<RegistryValueRecord> format(Map<String, Object> valuesByName) {
        List<String> names = new ArrayList<>(valuesByName.keySet());
        names.sort(Comparator.naturalOrder());

        List<RegistryValueRecord> rows = new ArrayList<>();
        for (String name : names) {
            Object v = valuesByName.get(name);
            String displayName = (name == null || name.isEmpty()) ? "(Default)" : name;
            rows.add(new RegistryValueRecord(displayName, typeLabel(v), formatData(v)));
        }
        return rows;
    }

    private static String typeLabel(Object value) {
        if (value == null) {
            return "REG_NONE";
        }
        if (value instanceof Integer) {
            return "REG_DWORD";
        }
        if (value instanceof Long) {
            return "REG_QWORD";
        }
        if (value instanceof String) {
            return "REG_SZ";
        }
        if (value instanceof byte[]) {
            return "REG_BINARY";
        }
        if (value instanceof String[]) {
            return "REG_MULTI_SZ";
        }
        return "REG_UNKNOWN";
    }

    private static String formatData(Object value) {
        if (value == null) {
            return "(empty)";
        }
        if (value instanceof String s) {
            return s.isEmpty() ? "(empty)" : s;
        }
        if (value instanceof Integer i) {
            long unsigned = Integer.toUnsignedLong(i);
            return String.format("0x%08X  (%d)", i, unsigned);
        }
        if (value instanceof Long l) {
            return String.format("0x%016X  (%d)", l, l);
        }
        if (value instanceof byte[] b) {
            if (b.length == 0) {
                return "(empty)";
            }
            StringBuilder sb = new StringBuilder(b.length * 3);
            for (int i = 0; i < b.length; i++) {
                if (i > 0) {
                    sb.append(' ');
                }
                sb.append(String.format("%02X", b[i]));
            }
            return sb.toString();
        }
        if (value instanceof String[] arr) {
            if (arr.length == 0) {
                return "(empty)";
            }
            return String.join("  |  ", arr);
        }
        return String.valueOf(value);
    }
}

