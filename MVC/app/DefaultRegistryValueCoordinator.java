package org.example.registryviewer.app;

import com.sun.jna.platform.win32.WinReg.HKEY;
import org.example.registryviewer.domain.RegistryValueRecord;
import org.example.registryviewer.registry.RegistryAccessException;
import org.example.registryviewer.registry.RegistryReadService;
import org.example.registryviewer.registry.RegistryValueDisplayFormatter;
import org.example.registryviewer.ui.RegistryView;

import javax.swing.table.DefaultTableModel;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.Map;

/**
 * Value read + format + table rendering flow.
 */
public final class DefaultRegistryValueCoordinator implements RegistryValueCoordinator {

    private static final DateTimeFormatter READY_TIME = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    private final RegistryReadService registryReadService;
    private final RegistryValueDisplayFormatter valueFormatter;

    /**
     * @param registryReadService abstraction for retrieving raw registry values
     * @param valueFormatter converts raw values into display records
     */
    public DefaultRegistryValueCoordinator(
            RegistryReadService registryReadService,
            RegistryValueDisplayFormatter valueFormatter) {
        this.registryReadService = registryReadService;
        this.valueFormatter = valueFormatter;
    }

    @Override
    public void loadValuesForKey(RegistryView view, HKEY hkey) {
        view.setStatus("Loading…");
        try {
            Map<String, Object> values = registryReadService.listValues(hkey);
            List<RegistryValueRecord> rows = valueFormatter.format(values);

            DefaultTableModel model = view.getValueTableModel();
            model.setRowCount(0);
            for (RegistryValueRecord r : rows) {
                model.addRow(new Object[]{r.getName(), r.getTypeDisplay(), r.getDataDisplay()});
            }

            String ready = "Ready  –  " + LocalDateTime.now().format(READY_TIME);
            view.setStatus(ready);
        } catch (RegistryAccessException ex) {
            view.setStatus("Error loading values: " + ex.getMessage());
        }
    }
}
