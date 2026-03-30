package org.example.registryviewer.app;

import com.sun.jna.platform.win32.WinReg.HKEY;
import org.example.registryviewer.config.AppConstants;
import org.example.registryviewer.domain.RegistryKeyNode;
import org.example.registryviewer.domain.RegistryValueRecord;
import org.example.registryviewer.domain.RootHive;
import org.example.registryviewer.registry.RegistryAccessException;
import org.example.registryviewer.registry.RegistryReadService;
import org.example.registryviewer.registry.RegistryValueDisplayFormatter;
import org.example.registryviewer.ui.RegistryTreeView;

import javax.swing.Timer;
import javax.swing.event.TreeExpansionEvent;
import javax.swing.event.TreeExpansionListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreePath;
import java.awt.event.ItemEvent;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;

/**
 * Coordinates user actions with the registry service and view (MVC controller).
 */
public final class RegistryController implements TreeSelectionListener, TreeExpansionListener {

    private static final DateTimeFormatter READY_TIME = DateTimeFormatter.ofPattern("yyyy-MM-dd HH:mm:ss");

    private final RegistryTreeView view;
    private final RegistryReadService registryReadService;
    private final RegistryValueDisplayFormatter valueFormatter;

    private final Timer refreshTimer;
    private HKEY currentHkey;

    public RegistryController(
            RegistryTreeView view,
            RegistryReadService registryReadService,
            RegistryValueDisplayFormatter valueFormatter) {
        this.view = view;
        this.registryReadService = registryReadService;
        this.valueFormatter = valueFormatter;

        this.refreshTimer = new Timer(
                (int) TimeUnit.SECONDS.toMillis(AppConstants.AUTO_REFRESH_INTERVAL_SEC),
                e -> loadValuesForCurrentKey()
        );
    }

    public void initialize() {
        populateRootKeys();

        view.getKeyTree().addTreeSelectionListener(this);
        view.getKeyTree().addTreeExpansionListener(this);
        view.getAutoRefreshToggle().addItemListener(this::onAutoRefreshToggle);

        view.setStatus("Ready  –  Select a key to begin.");
    }

    private void populateRootKeys() {
        DefaultMutableTreeNode root = view.getInvisibleTreeRoot();
        root.removeAllChildren();

        for (RootHive hive : RootHive.values()) {
            RegistryKeyNode nodeData = RegistryKeyNode.realKey(hive.getDisplayName(), hive.getDisplayName(), hive.getHkey());
            DefaultMutableTreeNode node = new DefaultMutableTreeNode(nodeData);
            node.add(new DefaultMutableTreeNode(RegistryKeyNode.placeholderChild()));
            root.add(node);
        }

        view.getTreeModel().reload();
    }

    private void onAutoRefreshToggle(ItemEvent e) {
        if (e.getStateChange() == ItemEvent.SELECTED) {
            refreshTimer.start();
            view.setStatus("Auto-refresh ON");
        } else {
            refreshTimer.stop();
            view.setStatus("Auto-refresh OFF");
        }
    }

    @Override
    public void valueChanged(TreeSelectionEvent e) {
        TreePath path = view.getKeyTree().getSelectionPath();
        if (path == null) {
            return;
        }

        DefaultMutableTreeNode treeNode = (DefaultMutableTreeNode) path.getLastPathComponent();
        Object uo = treeNode.getUserObject();
        if (!(uo instanceof RegistryKeyNode keyNode)) {
            return;
        }
        if (keyNode.isPlaceholder() || keyNode.getHkey() == null) {
            return;
        }

        currentHkey = keyNode.getHkey();
        view.setStatus("Viewing: " + keyNode.getFullPath());
        loadValuesForCurrentKey();
    }

    @Override
    public void treeExpanded(TreeExpansionEvent event) {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode) event.getPath().getLastPathComponent();
        Object uo = node.getUserObject();
        if (!(uo instanceof RegistryKeyNode data) || data.isPlaceholder()) {
            return;
        }

        // Only populate if the node still has a single "Loading..." placeholder child.
        if (node.getChildCount() == 0) {
            return;
        }

        DefaultMutableTreeNode first = (DefaultMutableTreeNode) node.getFirstChild();
        Object firstUo = first.getUserObject();
        if (!(firstUo instanceof RegistryKeyNode firstData) || !firstData.isLoadingPlaceholder()) {
            return;
        }

        node.removeAllChildren();

        HKEY parentHkey = data.getHkey();
        try {
            List<String> children = registryReadService.listSubkeyNames(parentHkey);
            for (String childName : children) {
                try {
                    HKEY childHkey = registryReadService.openChildReadOnly(parentHkey, childName);
                    String fullPath = data.getFullPath() + "\\" + childName;
                    DefaultMutableTreeNode child = new DefaultMutableTreeNode(
                            RegistryKeyNode.realKey(childName, fullPath, childHkey));
                    child.add(new DefaultMutableTreeNode(RegistryKeyNode.placeholderChild()));
                    node.add(child);
                } catch (RegistryAccessException ignored) {
                    // Same behavior as the C version: skip keys we cannot open.
                }
            }
        } catch (RegistryAccessException ex) {
            view.setStatus("Error listing subkeys: " + ex.getMessage());
        }

        view.getTreeModel().reload(node);
    }

    @Override
    public void treeCollapsed(TreeExpansionEvent event) {
        // no-op
    }

    private void loadValuesForCurrentKey() {
        if (currentHkey == null) {
            return;
        }

        view.setStatus("Loading…");
        try {
            Map<String, Object> values = registryReadService.listValues(currentHkey);
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

